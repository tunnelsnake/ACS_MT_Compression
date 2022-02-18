#ifndef COMMON_H
#define COMMON_H
#include <sstream>
#include <iomanip>
#include <exception>

std::string toHexStr(uint32_t *u)
{
    std::stringstream stream;
    stream << "0x" << std::hex << *u;
    std::string result(stream.str());
    return result;
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
