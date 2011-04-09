#ifndef _OPERAND_H_
#define _OPERAND_H_

class VM;

class Operand
{
public:
    int type, reg, dist, length, pc, size;

    void set(int t, int r, VM *vm, int ad, int sz);
};

#endif
