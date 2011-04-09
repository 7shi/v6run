#include "VM.h"

void Operand::set(int t, int r, VM *vm, int ad, int sz)
{
    type = t;
    reg = r;
    dist = length = 0;
    pc = ad;
    size = sz;
    if (t >= 6)
    {
        dist = vm->read16(ad);
        length = 2;
        pc += 2;
    }
    else if (r == 7 && (t == 2 || t == 3))
        length = 2;
}
