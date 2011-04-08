#ifndef _BINDATA_H_
#define _BINDATA_H_

#include <stddef.h>
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

#endif
