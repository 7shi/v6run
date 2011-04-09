#include "VM.h"

VM::VM(AOut *aout) : mem(65536)
{
    set(aout);
}

void VM::set(AOut *aout)
{
    this->aout = aout;
    isLong = isDouble = hasExited = Z = N = C = V = false;
    memcpy(&mem[0], &aout->image[0], aout->image.size());
    memset(r, 0, sizeof(r));
    r[7] = aout->entry;
    prevPC = 0;
    nextPC = indirBak = NULL;
    std::vector<std::string> args;
    args.push_back(aout->path);
    setArgs(args);
}

std::vector<std::string> VM::getArgs(int argc, int argv)
{
    std::vector<std::string> ret;
    int p;
    while (argc != 0 && (p = read16(argv)))
    {
        ret.push_back(getString(p));
        argc--;
        argv += 2;
    }
    return ret;
}

void VM::setArgs(const std::vector<std::string> &args)
{
    int p = 0x10000;
    std::vector<int> ptrs;
    for (int i = args.size() - 1; i >= 0; i--)
    {
        int alen = args[i].size();
        int blen = (alen / 2) * 2 + 2;
        p -= blen;
        memcpy(&mem[p], args[i].c_str(), alen);
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

void VM::run(const std::vector<std::string> &args)
{
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
