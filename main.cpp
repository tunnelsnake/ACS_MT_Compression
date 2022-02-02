#include "include/FileReader.hpp"
#include "include/ThreadSafeQueue.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <semaphore>
#include <utility>
#include <thread>

#define KB 1024
#define NBThreads 16

// Typedef this complex type so we are allowed to make std::shared_ptr to it.
// pair.first holds data and data size (vector), pair.second holds block index.
typedef std::pair<std::vector<uint8_t>, unsigned long> FileBlock;

void start_file_reader(ThreadSafeQueue<std::shared_ptr<FileBlock>> *q, FileReader * f) {
unsigned long block_index = 0;
 while (!f->eof()) {
       q->emplace(std::make_shared<FileBlock>(std::make_pair<std::vector<uint8_t>, unsigned long>(f->produce(), std::forward<unsigned long>(block_index))));
       block_index++;
    }

    std::cout << "Read " << q->size() << " blocks." << std::endl;
    unsigned int ctr = 1;
    size_t total = 0;
    while (q->size() != 0) {
        auto block = q->pop_back();
        std::cout << "\tBlock " << block->second << ": " << block->first.size() << std::endl;
        ctr++;
        total += block->first.size();
    }
    std::cout << "In total " << total << " bytes were read." << std::endl;
}

void start_compression_worker(ThreadSafeQueue<std::shared_ptr<FileBlock>> *q) {
    auto currentBlock = q->pop_back();
}

int main(int argc, char ** argv) {
    FileReader f("res/AlgoRecitation.pdf", 16 * KB);
    ThreadSafeQueue<std::shared_ptr<FileBlock>> q;

    std::thread FileWorker(start_file_reader, &q, &f);
    std::vector<std::thread> CompressionWorkers;
    for (int i = 0; i < NBThreads; i++) {
        CompressionWorkers.push_back(new std::thread(start_compression_worker, &q));
    }
    FileWorker.join();

    // start_file_reader(&q, &f);

    return 0;
}