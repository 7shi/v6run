#ifndef _AOUT_H_
#define _AOUT_H_

#include <stdint.h>
#include <string>
#include <vector>

class AOut
{
public:
    uint16_t fmagic, tsize, dsize, bsize, ssize, entry, pad, relflg;
    std::vector<uint8_t> image;
    std::string path;
    std::string error;

    AOut(const std::string &path);
    void set(const std::string &path);
};

#endif
