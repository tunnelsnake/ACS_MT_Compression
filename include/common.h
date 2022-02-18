#ifndef COMMON_H
#define COMMON_H
#include <sstream>
#include <iomanip>
#include <exception>
#include <climits>

void usage(char ** argv) {
    std::cerr << "Usage: " << argv[0] << " <compress | decompress> <input-path> <output-path> <num-workers> <compression-level>" << std::endl;
    exit(-1);
}

std::string toHexStr(uint32_t *u)
{
    std::stringstream stream;
    stream << "0x" << std::hex << *u;
    std::string result(stream.str());
    return result;
}

bool toInt(char * str, unsigned int * i) {
    char *p;
    errno = 0;
    long conv = strtol(str, &p, 10);

    // Check for errors: e.g., the string does not represent an integer
    // or the integer is larger than int
    if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN) {
        return false;
    } else {
        // No error
        *i = conv;
        return true;     
    }
}

class FileNotFoundException : public std::exception
{
    virtual const char *what() const throw()
    {
        return "The requested file was not found or could not be opened.";
    }
} FNFException;

class DecryptMagicNoException : public std::exception
{
    virtual const char *what() const throw()
    {
        return "The requested file was not compressed using this program or is corrupt.";
    }
} DMNException;

const uint32_t MAGIC_NO = 0xAABBCCDD;
#endif
