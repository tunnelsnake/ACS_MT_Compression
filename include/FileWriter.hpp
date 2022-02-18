#ifndef FILE_WRITER_H
#define FILE_WRITER_H
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <math.h>
#include "common.h"

inline bool checkFileExists(const std::string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

class FileWriter
{
public:
    FileWriter(std::string path, size_t block_size, size_t total_size, bool compress)
    {
        if (!checkFileExists(path))
        {
            this->_path = path;
            this->_f.open(path, std::ofstream::binary);
            this->_f.close();
        }
        std::filesystem::resize_file(path, total_size);
        this->_f.open(path, std::ofstream::binary);
        this->_bs = block_size;
        this->_bytes_written = 0;
        this->_total_size = total_size;
        this->_compress = compress;
        if (!_f.is_open())
        {
            throw FNFException;
        }
    }

    ~FileWriter()
    {
        // Write the offset table if in compression mode
        if (this->_compress) {
            this->_f.seekp(0);
            this->_f.write((const char *) &MAGIC_NO, sizeof(uint32_t));
            this->_f.write((const char *) &this->_nOffsetTableEntries, sizeof(uint32_t));
            this->_f.write((const char *) this->_offsetTable.data(), sizeof(uint32_t) * this->_nOffsetTableEntries);
        }
        if (_f.is_open())
        {
            _f.close();
        }
        std::cout << "Bytes Written: " << this->_bytes_written << "\n";
        std::cout << "Metadata Bytes: " << this->_offsetTable.size() * sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) << "\n";
        std::cout << std::flush;
        for (unsigned int i = 0; i < _offsetTable.size(); i++) {
            std::cout << _offsetTable[i] << std::endl;
        }
    }

    std::string *path()
    {
        return &_path;
    }

    size_t bytesWritten()
    {
        return _bytes_written;
    }

    size_t totalSize()
    {
        return _total_size;
    }

    void consume(void *data, size_t data_size)
    {
        // If we are compressing and this is the first block:
        // 'allocate' space in the file for the block offset table.
        if (this->_compress && this->_offsetTable.size() == 0) {
            this->_nOffsetTableEntries = ceil((double)this->_total_size / this->_bs);
            //                magic number       # table entries    table entries
            _bytes_written += sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) * this->_nOffsetTableEntries;
        }

        _f.seekp(_bytes_written);
        _f.write((const char *)data, data_size);
        _bytes_written += data_size;
        _offsetTable.push_back(data_size);
    }

private:
    std::ofstream _f;
    std::string _path;
    std::vector<uint32_t> _offsetTable;
    size_t _bs;
    size_t _bytes_written;
    size_t _total_size;
    size_t _nOffsetTableEntries;
    bool _compress;
};

#endif // FILE_WRITER_H