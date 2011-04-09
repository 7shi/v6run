#include "VM.h"

VM *VM::current;

void VM::runStep()
{
    current = this;
    if (nextPC)
    {
        r[7] = *nextPC;
        nextPC = NULL;
    }
    prevPC = r[7];
    if (prevPC & 1)
    {
        abort("invalid pc");
        return;
    }
}
