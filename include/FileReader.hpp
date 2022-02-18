#ifndef FILE_READER_H
#define FILE_READER_H
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <math.h>
#include "common.h"

class FileReader
{

public:
    FileReader(std::string path, unsigned int block_size, bool decompress)
    {
        _f.open(path, std::ifstream::binary);
        if (!_f.is_open())
            throw FNFException;
        _eof = _f.eof();
        _bs = block_size;
        _compressed = decompress;
        _nBlocksRead = 0;

        // If compressed, load our offset table scheme.
        if (_compressed)
        {
            uint32_t magic;
            uint32_t nOffsetTableEntries;
            _f.read((char *)&magic, sizeof(uint32_t));
            // If the magic number isn't ours, then don't decompress.
            if (magic != MAGIC_NO)
            {
                throw DMNException;
            }
            _f.read((char *)&nOffsetTableEntries, sizeof(uint32_t));
            this->_offsetTable.resize(nOffsetTableEntries);
            _f.read((char *)_offsetTable.data(), sizeof(uint32_t) * nOffsetTableEntries);
            // std::cout << "Offset Table Entries: " << nOffsetTableEntries << std::endl;
            int sum = 0;
            for (unsigned int i = 0; i < _offsetTable.size(); i++) {
                // std::cout << "\t" << _offsetTable.at(i) << "\n";
                sum += _offsetTable.at(i);
            }
            // std::cout << "Sum: " << sum << std::endl;
            // std::cout << std::flush;
        }
    }

    ~FileReader()
    {
        if (_f.is_open())
        {
            _f.close();
        }
    }

    size_t numBlocks() {
        return this->_compressed ? this->_offsetTable.size() : ceil((double)this->size() / this->_bs);
    }

    size_t size()
    {
        std::streampos cursor = _f.tellg();
        _f.seekg(0, _f.end);
        int length = _f.tellg();
        _f.seekg(cursor);
        return length;
    }

    std::vector<uint8_t> produce()
    {
        if (!_f.is_open())
        {
            std::cerr << "Error: File handle is no longer open!" << std::endl;
            exit(-1);
        }

        std::vector<uint8_t> b;
        size_t bytesToRead;
        if (!this->_compressed)
        {
            bytesToRead = _bs;
        }
        else
        {
            bytesToRead = this->_offsetTable.at(this->_nBlocksRead);
        }

        b.resize(bytesToRead);
        _f.read((char *)b.data(), bytesToRead);
        _eof = _f.eof();
        b.resize((size_t)_f.gcount());
        this->_nBlocksRead++;
        return b;
    }

    bool eof()
    {
        return _eof;
    }

private:
    std::ifstream _f;
    size_t _bs;
    bool _eof;
    bool _compressed;
    std::vector<uint32_t> _offsetTable;
    size_t _nBlocksRead;
};
#endif //FILE_READER_H
