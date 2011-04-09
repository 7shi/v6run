#include <list>
#include "AOut.h"
#include "Operand.h"
#include "utils.h"

class VM
{
private:
    AOut *aout;
    std::vector<uint8_t> mem;
    uint8_t prevPC;
    int pid;
    bool isLong, isDouble, hasExited;
    uint16_t r[8];
    bool Z, N, C, V;
    std::vector<int> fds;

public:
    VM(AOut *aout);
    VM(const std::string &cmd);

    inline uint8_t read8(int pos) { return mem[pos]; }
    inline void write8(int pos, uint8_t v) { mem[pos] = v; }
    inline uint16_t read16(int pos) { return readvec16(mem, pos); }
    inline void write16(int pos, uint16_t v) { writevec16(&mem, pos, v); }

    inline uint16_t getInc(int reg, int size)
    {
        int ret = r[reg];
        r[reg] += size;
        return ret;
    }

    inline uint16_t getDec(int reg, int size)
    {
        r[reg] -= size;
        return r[reg];
    }

    inline uint32_t getReg32(int reg)
    {
        return (r[reg] << 16) | r[(reg + 1) & 7];
    }

    inline void setReg32(int reg, uint32_t v)
    {
        r[reg] = v >> 16;
        r[(reg + 1) & 7] = v;
    }

    inline void setFlags(bool z, bool n, bool c, bool v)
    {
        Z = z;
        N = n;
        C = c;
        V = v;
    }

    void setArgs(std::list<std::string> args);
    void run(std::list<std::string> args);
    void run();
    void runStep();
    void abort(const std::string &msg);

private:
    void getSrcDst(Operand *src, Operand *dst, int size);
    void getDst(Operand *dst, int size, int len = 2);
};
