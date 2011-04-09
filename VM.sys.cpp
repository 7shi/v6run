#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include "VM.h"
#include "utils.h"

void VM::sys()
{
    int t = mem[r[7]];
    r[7] += 2;
    switch (t)
    {
        case  0: _indir (); return;
        case  1: _exit  (); return;
        case  2: _fork  (); return;
        case  3: _read  (); return;
        case  4: _write (); return;
        case  5: _open  (); return;
        case  6: _close (); return;
        case  7: _wait  (); return;
        case  8: _creat (); return;
        case  9: _link  (); return;
        case 10: _unlink(); return;
        case 11: _exec  (); return;
        case 12: _chdir (); return;
        case 13: _time  (); return;
        case 14: _mknod (); return;
        case 15: _chmod (); return;
        case 16: _chown (); return;
        case 17: _break (); return;
        case 18: _stat  (); return;
        case 19: _seek  (); return;
        case 20: _getpid(); return;
        case 21: _mount (); return;
        case 22: _umount(); return;
        case 23: _setuid(); return;
        case 24: _getuid(); return;
        case 25: _stime (); return;
        case 26: _ptrace(); return;
        case 28: _fstat (); return;
        case 31: _stty  (); return;
        case 32: _gtty  (); return;
        case 34: _nice  (); return;
        case 35: _sleep (); return;
        case 36: _sync  (); return;
        case 37: _kill  (); return;
        case 38: _switch(); return;
        case 41: _dup   (); return;
        case 42: _pipe  (); return;
        case 43: _times (); return;
        case 44: _prof  (); return;
        case 46: _setgid(); return;
        case 47: _getgid(); return;
        case 48: _signal(); return;
    }
    debug("unknown syscall: " + str(t));
    C = true;
}

void VM::_indir() // 0
{
    int tmp = read16(getInc(7, 2));
    uint16_t bak = r[7];
    indirBak = &bak;
    r[7] = tmp;
    sys();
    if (indirBak) r[7] = *indirBak;
}

void VM::_exit() // 1
{
    exit(r[0]);
}

void VM::_fork() // 2
{
    int result = fork();
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_read() // 3
{
    int fd = r[0];
    int bufp = read16(getInc(7, 2));
    int len  = read16(getInc(7, 2));
    int max = mem.size() - bufp;
    if (len > max) len = max;
    int result = read(fd, &mem[bufp], len);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_write() // 4
{
    int fd = r[0];
    int buf = read16(getInc(7, 2));
    int len = read16(getInc(7, 2));
    int max = mem.size() - buf;
    if (len > max) len = max;
    int result = write(fd, &mem[buf], len);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_open() // 5
{
    std::string path = getPath(getInc(7, 2));
    int mode = read16(getInc(7, 2));
    int result = open(path.c_str(), mode);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_close() // 6
{
    int fd = r[0];
    int result = close(fd);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_wait() // 7
{
    int status;
    int result = wait(&status);
    r[1] = status;
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_creat() // 8
{
    std::string path = getPath(getInc(7, 2));
    int mode = read16(getInc(7, 2));
    int result = creat(path.c_str(), mode);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_link() // 9
{
    std::string src = getPath(getInc(7, 2));
    std::string dst = getPath(getInc(7, 2));
    int result = link(src.c_str(), dst.c_str());
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_unlink() // 10
{
    std::string path = getPath(getInc(7, 2));
    int result = unlink(path.c_str());
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_exec() // 11
{
    std::string path = getPath(getInc(7, 2));
    std::vector<std::string> args = getArgs(-1, read16(getInc(7, 2)));
    AOut aout(path);
    if (!aout.image.empty())
    {
        *this->aout = aout;
        set(this->aout);
        setArgs(args);
    }
    else
    {
        std::vector<const char *> argv;
        for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it)
            argv.push_back(it->c_str());
        argv.push_back(NULL);
        int result = execv(path.c_str(), const_cast<char *const *>(&argv[0]));
        r[0] = (C = (result == -1)) ? errno : result;
    }
}

void VM::_chdir() // 12
{
    std::string path = getPath(getInc(7, 2));
    int result = chdir(path.c_str());
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_time() // 13
{
    time_t result = time(NULL);
    r[0] = result >> 16;
    r[1] = result;
    C = false;
}

void VM::_mknod() // 14
{
    r[7] += 6;
    debug("sys mknod: not implemented");
    C = true;
}

void VM::_chmod() // 15
{
    std::string path = getPath(getInc(7, 2));
    int mode = read16(getInc(7, 2));
    int result = chmod(path.c_str(), mode);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_chown() // 16
{
    r[7] += 4;
    debug("sys chown: not implemented");
    C = true;
}

void VM::_break() // 17
{
    int nd = read16(getInc(7, 2));
    if (nd < aout->tsize + aout->dsize || nd >= r[6])
    {
        r[0] = 0;
        C = true;
    }
    else
        C = false;
}

void VM::_stat() // 18
{
    std::string path = getPath(getInc(7, 2));
    int *pst = (int *)&mem[read16(getInc(7, 2))];
    struct stat st;
    int result = stat(path.c_str(), &st);
    if (C = (result == -1))
        r[0] = errno;
    else
    {
        // TODO: convert stat
        pst[0] = 0;
        r[0] = result;
    }
}

void VM::_seek() // 19
{
    int fd = r[0];
    int p = read16(getInc(7, 2));
    int t = read16(getInc(7, 2));
    int result;
    switch (t)
    {
        case 0 : result = lseek(fd, p, SEEK_SET); break;
        case 1 : result = lseek(fd, int16_t(uint16_t(p)), SEEK_CUR); break;
        case 2 : result = lseek(fd, int16_t(uint16_t(p)), SEEK_END); break;
        case 3 : result = lseek(fd, p * 512, SEEK_SET); break;
        case 4 : result = lseek(fd, int(int16_t(uint16_t(p))) * 512, SEEK_CUR); break;
        default: result = lseek(fd, int(int16_t(uint16_t(p))) * 512, SEEK_END); break;
    }
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_getpid() // 20
{
    int result = getpid();
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_mount() // 21
{
    r[7] += 6;
    debug("sys mount: not implemented");
    C = true;
}

void VM::_umount() // 22
{
    r[7] += 2;
    debug("sys umount: not implemented");
    C = true;
}

void VM::_setuid() // 23
{
    debug("sys setuid: not implemented");
    C = true;
}

void VM::_getuid() // 24
{
    debug("sys getuid: not implemented");
    C = true;
}

void VM::_stime() // 25
{
    debug("sys getuid: not implemented");
    C = true;
}

void VM::_ptrace() // 26
{
    r[7] += 6;
    debug("sys ptrace: not implemented");
    C = true;
}

void VM::_fstat() // 28
{
    r[7] += 2;
    debug("sys fstat: not implemented");
    C = true;
}

void VM::_stty() // 31
{
    r[7] += 2;
    debug("sys stty: not implemented");
    C = true;
}

void VM::_gtty() // 32
{
    r[7] += 2;
    debug("sys gtty: not implemented");
    C = true;
}

void VM::_nice() // 34
{
    debug("sys nice: not implemented");
    C = true;
}

void VM::_sleep() // 35
{
    int result = sleep(r[0]);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_sync() // 36
{
    debug("sys sync: not implemented");
    C = true;
}

void VM::_kill() // 37
{
    r[7] += 2;
    debug("sys kill: not implemented");
    C = true;
}

void VM::_switch() // 38
{
    debug("sys switch: not implemented");
    C = true;
}

void VM::_dup() // 41
{
    int result = dup(r[0]);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_pipe() // 42
{
    debug("sys pipe: not implemented");
    C = true;
}

void VM::_times() // 43
{
    r[7] += 2;
    debug("sys times: not implemented");
    C = true;
}

void VM::_prof() // 44
{
    r[7] += 8;
    debug("sys prof: not implemented");
    C = true;
}

void VM::_setgid() // 46
{
    debug("sys setgid: not implemented");
    C = true;
}

void VM::_getgid() // 47
{
    debug("sys getgid: not implemented");
    C = true;
}
