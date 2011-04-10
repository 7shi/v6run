#ifndef _OPERAND_H_
#define _OPERAND_H_

#include <sys/types.h>
#include <string>

class VM;

class Operand
{
private:
    VM *vm;

public:
    int type, reg, dist, length, pc, size;

public:
    void set(VM *vm, int t, int r, int ad, int sz);
    uint16_t getAddress();
    uint16_t getValue(bool nomove = false);
    void setValue(uint16_t v);
    uint8_t getByte(bool nomove = false);
    void setByte(uint8_t v);
    std::string tos() const;
};

#endif
