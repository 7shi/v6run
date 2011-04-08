#ifndef _BINDATA_H_
#define _BINDATA_H_

#include <stddef.h>
#include <vector>

class BinData
{
public:
    std::vector<uint8_t> Data;
    bool UseOct;

    BinData(int size);

    inline uint16_t Read16(int pos)
    {
        return Data[pos] | (Data[pos + 1] << 8);
    }

    inline void Write16(int pos, uint16_t v)
    {
        Data[pos] = static_cast<uint8_t>(v);
        Data[pos + 1] = static_cast<uint8_t>(v >> 8);
    }
};

#endif
