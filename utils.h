#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <vector>
#include <string>

std::string hex(int v);
void setroot(std::string root);
std::string convpath(const std::string &path);
bool startsWith(const std::string &s, const std::string &prefix);
bool endsWith(const std::string &s, const std::string &prefix);

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
