#ifndef _UTILS_H_
#define _UTILS_H_

#include <cstddef>
#include <vector>
#include <string>

std::string hex(int v);

inline uint16_t readvec16(const std::vector<uint8_t> &vec, int pos)
{
    return vec[pos] | (vec[pos + 1] << 8);
}

inline void writevec16(std::vector<uint8_t> *vec, int pos, uint16_t v)
{
    (*vec)[pos] = v;
    (*vec)[pos + 1] = v >> 8;
}

#endif
