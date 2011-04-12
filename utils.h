#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <vector>
#include <string>

extern std::string regnames[];

std::string hex(int v);
std::string oct(int v, int len = 0);
std::string str(int v);
std::string sdist(int d);
void setroot(std::string root);
std::string convpath(const std::string &path);
bool startsWith(const std::string &s, const std::string &prefix);
bool endsWith(const std::string &s, const std::string &prefix);
std::string replace(const std::string &src, const std::string &s1, const std::string &s2);

inline uint16_t readvec16(const std::vector<uint8_t> &vec, uint16_t pos)
{
    return vec[pos] | (vec[pos + 1] << 8);
}

inline void writevec16(std::vector<uint8_t> *vec, uint16_t pos, uint16_t v)
{
    (*vec)[pos] = v;
    (*vec)[pos + 1] = v >> 8;
}

#ifdef WIN32
std::string getErrorMessage(int err);
#endif

#endif
