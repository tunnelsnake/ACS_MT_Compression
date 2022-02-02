#ifndef FILE_READER_H
#define FILE_READER_H
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

class FileReader
{

public:
    FileReader(std::string path, unsigned int block_size)
    {
        this->_f.open(path, std::ifstream::binary);
        this->_eof = _f.eof();
        this->_bs = block_size;
    }

    ~FileReader()
    {
        if (!_f.is_open())
        {
            _f.close();
        }
    }

    std::vector<uint8_t> produce()
    {
        if (!_f.is_open())
        {
            std::cerr << "Error: File handle is no longer open!" << std::endl;
            exit(-1);
        }

        std::vector<uint8_t> b(this->_bs);
        this->_f.read((char *) b.data(), this->_bs);
        this->_eof = _f.eof();
        b.resize((size_t) _f.gcount());
        return b;
    }

    bool eof() {
        return _eof;
    }

private:
    std::ifstream _f;
    size_t _bs;
    bool _eof;
};
#endif //FILE_READER_H
