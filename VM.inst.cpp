#include "VM.h"

void VM::runStep()
{
    prevPC = r[7];
    if (prevPC & 1)
    {
        abort("invalid pc");
        return;
    }
}
