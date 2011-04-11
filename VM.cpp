#include <cstring>
#include <cstdio>
#include "VM.h"

#ifdef WIN32
VM::VM() {}
#endif

VM::VM(AOut *aout) : mem(65536), trace(0)
{
    set(aout);
}

void VM::set(AOut *aout)
{
    this->aout = aout;
    isLong = isDouble = hasExited = Z = N = C = V = false;
    memset(r, 0, sizeof(r));
    memset(&mem[0], 0, mem.size());
    memcpy(&mem[0], &aout->image[0], aout->image.size());
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
        ret.push_back(readstr(p));
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
    debug(msg);
    hasExited = true;
}

void VM::getSrcDst(Operand *src, Operand *dst, int size)
{
    uint16_t v = read16(r[7]);
    src->set(this, (v >> 9) & 7, (v >> 6) & 7, r[7] + 2, size);
    getDst(dst, size, src->length + 2);
}

void VM::getDst(Operand *dst, int size, int len)
{
    uint16_t v = read16(r[7]);
    dst->set(this, (v >> 3) & 7, v & 7, r[7] + len, size);
    r[7] += len + dst->length;
}

int VM::getOffset(int pos)
{
    int d = int8_t(read8(pos));
    return pos + 2 + d * 2;
}

void VM::debug(const std::string &msg)
{
    fprintf(stderr,
        "%04x,%04x,%04x,%04x,%04x,%04x,sp=%04x,pc=%04x: %s\n",
        r[0], r[1], r[2], r[3], r[4], r[5], r[6], prevPC, msg.c_str());
}

void VM::debug(const std::string &op, const Operand &dst)
{
    debug(op + " " + dst.tos());
}

void VM::debug(const std::string &op, const Operand &src, const Operand &dst)
{
    debug(op + " " + src.tos() + ", " + dst.tos());
}

void VM::debug(const std::string &op, int reg)
{
    debug(op + " " + regnames[reg]);
}

void VM::debug(const std::string &op, int reg, const Operand &dst)
{
    debug(op + " " + regnames[reg] + ", " + dst.tos());
}

void VM::debug(const std::string &op, const Operand &dst, int reg)
{
    debug(op + " " + dst.tos() + ", " + regnames[reg]);
}
