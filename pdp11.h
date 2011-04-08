#ifndef _PDP11_H_
#define _PDP11_H_

#include <cstddef>
#include <string>
#include <vector>

inline uint16_t read16(const std::vector<uint8_t> &vec, int pos)
{
    return vec[pos] | (vec[pos + 1] << 8);
}

inline void write16(std::vector<uint8_t> *vec, int pos, uint16_t v)
{
    (*vec)[pos] = static_cast<uint8_t>(v);
    (*vec)[pos + 1] = static_cast<uint8_t>(v >> 8);
}

class AOut
{
public:
    uint16_t fmagic, tsize, dsize, bsize, ssize, entry, pad, relflg;
    std::vector<uint8_t> image;

    AOut(const std::string &file);
};

#endif
