#include "include/FileReader.hpp"
#include "include/FileWriter.hpp"
#include "include/ThreadSafeQueue.hpp"
#include "include/ThreadSafePriorityQueue.hpp"
#include "include/util.h"
#include "zstd/lib/zstd.h"
#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <thread>
#include <chrono>
#include <queue>
#include <cmath>
#include <cstring>
#include <tuple>

#define KB 1024
#define MODE_COMPRESSION 0
#define MODE_DECOMPRESSION 1

unsigned int compressLevel;
unsigned int nThreads = 16;

// Typedef this complex type so we are allowed to make std::shared_ptr to it.
// pair.first holds data and data size (vector), pair.second holds block index.
typedef std::pair<std::vector<uint8_t>, unsigned long> FileBlock;
// Tuple: Same layout as above, with uncompressed size as last index
typedef std::tuple<std::vector<uint8_t>, unsigned long, unsigned long> CompressedBlock;

void start_file_reader(ThreadSafeQueue<std::shared_ptr<FileBlock>> *q, FileReader *f)
{
    unsigned long block_index = 0;
    size_t numBlocks = f->numBlocks();
    while (!f->eof() && (block_index != numBlocks))
    {
        q->emplace(std::make_shared<FileBlock>(std::make_pair<std::vector<uint8_t>, unsigned long>(f->produce(), std::forward<unsigned long>(block_index))));
        block_index++;
    }
}
template <class T, class Container, class Compare>
void start_file_writer(ThreadSafePriorityQueue<T, Container, Compare> *p, FileWriter *f, size_t *nBlocksWritten, size_t *nBytesWritten, bool *forceExit)
{
    unsigned int block_index = 0;
    *nBytesWritten = 0;
    // std::cout << f->bytesWritten() << f->totalSize() << std::endl;
    while (*forceExit != true)
    {
        std::optional<std::shared_ptr<CompressedBlock>> b = p->try_pop_front();
        // Write the block to a file
        if (b.has_value())
        {
            if (std::get<1>(*b.value()) == block_index)
            {
                f->consume(std::get<0>(*b.value()).data(), std::get<0>(*b.value()).size());
                // std::cout << "Writer: Block Hit " << block_index << " " << std::get<2>(*b.value()) << std::endl;
                block_index++;
                *nBlocksWritten = *nBlocksWritten + 1;
                *nBytesWritten = f->bytesWritten();
            }
            else
            {
                // std::cout << "Writer: Block Miss!" << block_index << " " << p->size() << std::endl;
                p->emplace(b.value());
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}

template <class T, class Container, class Compare>
void start_compression_worker(ThreadSafeQueue<std::shared_ptr<FileBlock>> *FileBlockQueue, ThreadSafePriorityQueue<T, Container, Compare> *CompressedBlockQueue, uint8_t MODE, bool *forceExit)
{

    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    while (*forceExit != true)
    {
        std::optional<std::shared_ptr<FileBlock>> b = FileBlockQueue->try_pop_front();
        if (b.has_value())
        {
            unsigned long block_index = b.value()->second;

            if (MODE == MODE_COMPRESSION)
            {
                // Compress the block, resizing vectors as necessary.
                unsigned int uncompressedSize = b.value()->first.size();
                unsigned int compressBound = ZSTD_compressBound(uncompressedSize);
                std::shared_ptr<CompressedBlock> compressed = std::make_shared<CompressedBlock>(std::make_tuple<std::vector<uint8_t>, unsigned long, unsigned long>(std::vector<uint8_t>(compressBound), std::forward<unsigned long>(block_index), std::forward<unsigned long>(uncompressedSize)));
                size_t const compressedSize = ZSTD_compressCCtx(cctx, std::get<0>(*compressed).data(), std::get<0>(*compressed).size(), b.value()->first.data(), b.value()->first.size(), compressLevel);
                CHECK_ZSTD(compressedSize);

                std::get<0>(*compressed).resize(compressedSize);
                CompressedBlockQueue->emplace(compressed);
            }
            else if (MODE == MODE_DECOMPRESSION)
            {
                // std::cout << b.value()->first.size() << "\n";
                // uint32_t *block_magic = (uint32_t *)b.value()->first.data();
                // std::cout << "Block magic " << toHexStr(block_magic) << std::endl;
                unsigned long long compressedSize = b.value()->first.size();
                unsigned long long const decompressedSize = ZSTD_getFrameContentSize(b.value()->first.data(), b.value()->first.size());
                // std::cout << "decompressedSize " << decompressedSize << " " << ZSTD_CONTENTSIZE_ERROR << std::endl;
                std::shared_ptr<CompressedBlock> decompressed = std::make_shared<CompressedBlock>(std::make_tuple<std::vector<uint8_t>, unsigned long, unsigned long>(std::vector<uint8_t>(decompressedSize), std::forward<unsigned long>(block_index), std::forward<unsigned long>(compressedSize)));
                CHECK(decompressedSize != ZSTD_CONTENTSIZE_ERROR, " not compressed by zstd!");
                CHECK(decompressedSize != ZSTD_CONTENTSIZE_UNKNOWN, " original size unknown!");
                size_t const dSize = ZSTD_decompress(std::get<0>(*decompressed).data(), std::get<0>(*decompressed).size(), b.value()->first.data(), b.value()->first.size());
                // std::cout << dSize << std::endl;
                CHECK_ZSTD(dSize);
                /* When zstd knows the content size, it will error if it doesn't match. */
                CHECK(decompressedSize == dSize, "Impossible because zstd will check this condition!");
                CompressedBlockQueue->emplace(decompressed);
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 6)
        usage(argv);
    if (strcmp(argv[1], "compress") != 0 && strcmp(argv[1], "decompress") != 0 && toInt(argv[4], &compressLevel) && toInt(argv[5], &nThreads))
    {
        usage(argv);
    }
    // std::cout << "mode: " << argv[1] << std::endl;
    // std::cout << "input-path: " << argv[2] << std::endl;
    // std::cout << "output-path: " << argv[3] << std::endl;
    std::cout << "Running with options:\n";
    std::cout << "\tinput: " << argv[2] << "\n";
    std::cout << "\toutput: " << argv[3] << "\n";
    std::cout << "\tcLevel: " << argv[4] << "\n";
    std::cout << "\tnThreads: " << argv[5] << "\n" << std::flush;
    auto begin = std::chrono::high_resolution_clock::now();

    unsigned long fileblock_size = 16 * KB;
    bool compress = strcmp(argv[1], "compress") == 0 ? true : false;
    FileReader r(argv[2], fileblock_size, !compress);
    size_t uncompressedSize = r.size();
    size_t numBlocksTotal = r.numBlocks();
    FileWriter w(argv[3], fileblock_size, uncompressedSize, compress);
    // std::cout << "Total # Blocks " << numBlocksTotal << std::endl;
    // std::cout << "Total File Size " << r.size() << std::endl;

    // Technically this compare is backwards from traditional high-prio-first.  Uses lowest-prio-first.
    auto compare = [](std::shared_ptr<CompressedBlock> lhs, std::shared_ptr<CompressedBlock> rhs)
    {
        // return lhs->second > rhs->second;
        return std::get<1>(*lhs) > std::get<1>(*rhs);
    };

    ThreadSafeQueue<std::shared_ptr<FileBlock>> FileBlockQueue;
    ThreadSafePriorityQueue<std::shared_ptr<CompressedBlock>, std::vector<std::shared_ptr<CompressedBlock>>, decltype(compare)> CompressedBlockQueue(compare);

    bool forceWorkersToExit = false;
    size_t numBlocksOut = 0;
    size_t compressedSize = 0;
    std::thread IFileWorker(start_file_reader, &FileBlockQueue, &r);
    std::thread OFileWorker(
        start_file_writer<std::shared_ptr<CompressedBlock>, std::vector<std::shared_ptr<CompressedBlock>>, decltype(compare)>,
        &CompressedBlockQueue,
        &w,
        &numBlocksOut,
        &compressedSize,
        &forceWorkersToExit);

    std::vector<std::thread> CompressionWorkers;
    for (unsigned int i = 0; i < nThreads; i++)
    {
        CompressionWorkers.push_back(std::thread(
            start_compression_worker<std::shared_ptr<CompressedBlock>, std::vector<std::shared_ptr<CompressedBlock>>, decltype(compare)>,
            &FileBlockQueue,
            &CompressedBlockQueue,
            strcmp(argv[1], "compress") == 0 ? MODE_COMPRESSION : MODE_DECOMPRESSION,
            &forceWorkersToExit));
    }

    // Wait until IFileWorker, OFileWorker and Compression Workers have finished.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    while (numBlocksTotal != numBlocksOut)
        ;
    forceWorkersToExit = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Join all the threads we created since the workload is complete.
    IFileWorker.join();
    OFileWorker.join();
    for (unsigned int i = 0; i < nThreads; i++)
    {
        CompressionWorkers[i].join();
    }
    // std::cout << "Compressed " << uncompressedSize << " down to " << compressedSize << " bytes." << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    std::cerr << "time: " << ns.count() << " nanoseconds" << '\n' << std::flush;

    return 0;
}