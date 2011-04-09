#include <vector>
#include "AOut.h"
#include "Operand.h"
#include "utils.h"

class VM
{
public:
    uint16_t r[8];

private:
    static VM *current;
    AOut *aout;
    std::vector<uint8_t> mem;
    uint16_t prevPC, *nextPC, *indirBak;
    bool isLong, isDouble, hasExited;
    bool Z, N, C, V;

public:
    bool verbose;

    VM(AOut *aout);
    void set(AOut *aout);

    inline uint8_t read8(uint16_t pos) { return mem[pos]; }
    inline void write8(uint16_t pos, uint8_t v) { mem[pos] = v; }
    inline uint16_t read16(uint16_t pos) { return readvec16(mem, pos); }
    inline void write16(uint16_t pos, uint16_t v) { writevec16(&mem, pos, v); }

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

    inline std::string getString(uint16_t pos)
    {
        return (const char *)&mem[pos];
    }

    inline std::string getPath(uint16_t pos)
    {
        return convpath(getString(pos));
    }

    inline void setFlags(bool z, bool n, bool c, bool v)
    {
        Z = z;
        N = n;
        C = c;
        V = v;
    }

    std::vector<std::string> getArgs(int argc, int argv);
    void setArgs(const std::vector<std::string> &args);
    void run(const std::vector<std::string> &args);
    void run();
    void runStep();
    void abort(const std::string &msg);

private:
    void exec0();
    void exec7();
    void exec10();
    void exec17();
    void getSrcDst(Operand *src, Operand *dst, int size);
    void getDst(Operand *dst, int size, int len = 2);
    int getOffset(int pos);

    void sys();
    void _indir (); //  0
    void _exit  (); //  1
    void _fork  (); //  2
    void _read  (); //  3
    void _write (); //  4
    void _open  (); //  5
    void _close (); //  6
    void _wait  (); //  7
    void _creat (); //  8
    void _link  (); //  9
    void _unlink(); // 10
    void _exec  (); // 11
    void _chdir (); // 12
    void _time  (); // 13
    void _mknod (); // 14
    void _chmod (); // 15
    void _chown (); // 16
    void _break (); // 17
    void _stat  (); // 18
    void _seek  (); // 19
    void _getpid(); // 20
    void _mount (); // 21
    void _umount(); // 22
    void _setuid(); // 23
    void _getuid(); // 24
    void _stime (); // 25
    void _ptrace(); // 26
    void _fstat (); // 28
    void _stty  (); // 31
    void _gtty  (); // 32
    void _nice  (); // 34
    void _sleep (); // 35
    void _sync  (); // 36
    void _kill  (); // 37
    void _switch(); // 38
    void _dup   (); // 41
    void _pipe  (); // 42
    void _times (); // 43
    void _prof  (); // 44
    void _setgid(); // 46
    void _getgid(); // 47
    void _signal(); // 48
    static void sighandler(int sig);

public:
    void debug(const std::string &msg);
    void debug(const std::string &op, const Operand &dst);
    void debug(const std::string &op, const Operand &src, const Operand &dst);
    void debug(const std::string &op, int reg);
    void debug(const std::string &op, int reg, const Operand &dst);
    void debug(const std::string &op, const Operand &dst, int reg);
};
