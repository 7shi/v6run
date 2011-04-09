#include "VM.h"

VM::VM(AOut *aout) : aout(aout), mem(65536)
{
    isLong = isDouble = hasExited = Z = N = C = V = false;
    memset(r, 0, sizeof(r));
    memcpy(&mem[0], &aout->image[0], aout->image.size());
    prevPC = 0;
    nextPC = NULL;
    std::list<std::string> args;
    args.push_back(aout->path);
    setArgs(args);
}

void VM::setArgs(std::list<std::string> args)
{
    int p = 0x10000;
    std::vector<int> ptrs;
    args.reverse();
    for (std::list<std::string>::iterator it = args.begin(); it != args.end(); ++it)
    {
        int alen = it->size();
        int blen = (alen / 2) * 2 + 2;
        p -= blen;
        memcpy(&mem[p], it->c_str(), alen);
        memset(&mem[p + alen], 0, blen - alen);
        ptrs.push_back(p);
    }
    for (std::vector<int>::iterator it = ptrs.begin(); it != ptrs.end(); ++it)
    {
        p -= 2;
        write16(p, *it);
    }
    p -= 2;
    write16(p, args.size());
    r[6] = p;
}

void VM::run(std::list<std::string> args)
{
    args.push_front(aout->path);
    setArgs(args);
    run();
}

void VM::run()
{
    hasExited = false;
    while (!hasExited) runStep();
}

void VM::abort(const std::string &msg)
{
    fprintf(stderr, "pc=%04x: %s\n", prevPC, msg.c_str());
    hasExited = true;
}

void VM::getSrcDst(Operand *src, Operand *dst, int size)
{
    uint16_t v = read16(r[7]);
    src->set((v >> 9) & 7, (v >> 6) & 7, this, r[7] + 2, size);
    getDst(dst, size, src->length + 2);
}

void VM::getDst(Operand *dst, int size, int len)
{
    uint16_t v = read16(r[7]);
    dst->set((v >> 3) & 7, v & 7, this, r[7] + len, size);
    r[7] += len + dst->length;
}
