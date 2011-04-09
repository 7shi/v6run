#ifndef _OPERAND_H_
#define _OPERAND_H_

#include <sys/types.h>

class VM;

class Operand
{
public:
    int type, reg, dist, length, pc, size;

    void set(int t, int r, VM *vm, int ad, int sz);
    uint16_t getAddress(VM *vm);
    uint16_t getValue(VM *vm, bool nomove = false);
    void setValue(VM *vm, uint16_t v);
    uint8_t getByte(VM *vm, bool nomove = false);
    void setByte(VM *vm, uint8_t v);
};

#endif
