#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#define NO_FORK
#else
#include <sys/wait.h>
#endif
#include <stack>
#include <map>
#include <algorithm>
#include "VM.h"
#include "utils.h"

#ifdef NO_FORK
static std::stack<uint16_t> exitcodes;
#endif
std::map<int, std::string> fd2name;

#ifdef WIN32
std::list<std::string> unlinks;

static void showError(int err)
{
    fprintf(stderr, "%s", getErrorMessage(err).c_str());
}

static void procResult(VM *vm, bool ok)
{
    vm->C = !ok;
    if (ok)
        vm->r[0] = 0;
    else
    {
        int err = GetLastError();
        vm->r[0] = err;
        if (vm->trace) showError(err);
    }
}
#endif

static int fileClose(VM *vm, int fd)
{
    int ret = close(fd);
    std::map<int, std::string>::iterator it = fd2name.find(fd);
    if (it != fd2name.end())
    {
        std::string path = it->second;
        fd2name.erase(it);
#ifdef WIN32
        std::list<std::string>::iterator it2 =
            std::find(unlinks.begin(), unlinks.end(), path);
        if (it2 != unlinks.end())
        {
            if (vm->trace)
                fprintf(stderr, "delayed unlink: %s\n", path.c_str());
            if (DeleteFileA(path.c_str()))
                unlinks.erase(it2);
            else if (vm->trace)
                showError(GetLastError());
        }
#endif
    }
    return ret;
}

VM::syshandler VM::syscalls[nsyscalls] = {
    { "indir"  , &VM::_indir   }, //  0
    { "exit"   , &VM::_exit    }, //  1
    { "fork"   , &VM::_fork    }, //  2
    { "read"   , &VM::_read    }, //  3
    { "write"  , &VM::_write   }, //  4
    { "open"   , &VM::_open    }, //  5
    { "close"  , &VM::_close   }, //  6
    { "wait"   , &VM::_wait    }, //  7
    { "creat"  , &VM::_creat   }, //  8
    { "link"   , &VM::_link    }, //  9
    { "unlink" , &VM::_unlink  }, // 10
    { "exec"   , &VM::_exec    }, // 11
    { "chdir"  , &VM::_chdir   }, // 12
    { "time"   , NULL          }, // 13
    { "mknod"  , NULL          }, // 14
    { "chmod"  , &VM::_chmod   }, // 15
    { "chown"  , NULL          }, // 16
    { "break"  , &VM::_break   }, // 17
    { "stat"   , &VM::_stat    }, // 18
    { "seek"   , &VM::_seek    }, // 19
    { "getpid" , &VM::_getpid  }, // 20
    { "mount"  , NULL          }, // 21
    { "umount" , NULL          }, // 22
    { "setuid" , NULL          }, // 23
    { "getuid" , NULL          }, // 24
    { "stime"  , NULL          }, // 25
    { "ptrace" , NULL          }, // 26
    { NULL     , NULL          }, // 27
    { "fstat"  , NULL          }, // 28
    { NULL     , NULL          }, // 29
    { NULL     , NULL          }, // 30
    { "stty"   , NULL          }, // 31
    { "gtty"   , NULL          }, // 32
    { NULL     , NULL          }, // 33
    { "nice"   , NULL          }, // 34
    { "sleep"  , NULL          }, // 35
    { "sync"   , NULL          }, // 36
    { "kill"   , NULL          }, // 37
    { "switch" , NULL          }, // 38
    { NULL     , NULL          }, // 39
    { NULL     , NULL          }, // 40
    { "dup"    , &VM::_dup     }, // 41
    { "pipe"   , NULL          }, // 42
    { "times"  , NULL          }, // 43
    { "prof"   , NULL          }, // 44
    { NULL     , NULL          }, // 45
    { "setgid" , NULL          }, // 46
    { "getgid" , NULL          }, // 47
    { "signal" , &VM::_signal  }, // 48
};

void VM::sys()
{
    int type = mem[r[7]];
    r[7] += 2;
    if (type < nsyscalls) {
        syshandler *sh = &syscalls[type];
        if (sh->name) {
            if (trace || !sh->f) {
                fprintf(stderr, "system call %s", sh->name);
            }
            if (sh->f) {
                (this->*sh->f)();
            } else {
                fprintf(stderr, ": not implemented\n");
                C = true;
            }
            return;
        }
    }
    fprintf(stderr, "system call %d: unknown\n", type);
    C = true;
}

void VM::_indir() // 0
{
    int tmp = read16(getInc(7, 2));
    if (trace) debug("sys indir; " + hex(tmp));
    uint16_t bak = r[7];
    indirBak = &bak;
    r[7] = tmp;
    sys();
    if (indirBak) r[7] = *indirBak;
}

void VM::_exit() // 1
{
    if (trace) debug("sys exit");
    exitcode = r[0];
#ifdef NO_FORK
    exitcodes.push(exitcode);
#endif
    hasExited = true;
    for (std::list<int>::iterator it = handles.begin(); it != handles.end(); ++it)
        fileClose(this, *it);
    handles.clear();
}

void VM::_fork() // 2
{
    if (trace) debug("sys fork");
#ifdef NO_FORK
    VM vm = *this;
    vm.handles.clear();
    vm.run();
    r[0] = 1;
    r[7] += 2;
    C = false;
#else
    int result = fork();
    r[0] = (C = (result == -1)) ? errno : result;
    if (!C && result) r[7] += 2;
#endif
}

void VM::_read() // 3
{
    int fd = r[0];
    int buf = read16(getInc(7, 2));
    int len  = read16(getInc(7, 2));
    if (trace) debug("sys read; " + hex(buf) + "; " + hex(len));
    int max = mem.size() - buf;
    if (len > max) len = max;
    int result = read(fd, &mem[buf], len);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_write() // 4
{
    int fd = r[0];
    int buf = read16(getInc(7, 2));
    int len = read16(getInc(7, 2));
    if (trace) debug("sys write; " + hex(buf) + "; " + hex(len));
    int max = mem.size() - buf;
    if (len > max) len = max;
    int result = write(fd, &mem[buf], len);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_open() // 5
{
    std::string path = readstrp(getInc(7, 2));
    int flag = read16(getInc(7, 2));
    if (trace) debug("sys open; \"" + path + "\"; " + str(flag));
    std::string path2 = convpath(path);
#if WIN32
    flag |= O_BINARY;
#endif
    int result = open(path2.c_str(), flag);
    r[0] = (C = (result == -1)) ? errno : result;
    if (!C)
    {
        fd2name[result] = path2;
        handles.push_back(result);
    }
}

void VM::_close() // 6
{
    int fd = r[0];
    if (trace) debug("sys close");
    int result = fileClose(this, fd);
    r[0] = (C = (result == -1)) ? errno : result;
    if (!C) handles.remove(result);
}

void VM::_wait() // 7
{
    if (trace) debug("sys wait");
#ifdef NO_FORK
    C = exitcodes.empty();
    if (!C)
    {
        r[1] = (exitcodes.top() << 8) | 14; // status
        r[0] = 1;
        exitcodes.pop();
    }
#else
    int status;
    int result = wait(&status);
    r[1] = status;
    r[0] = (C = (result == -1)) ? errno : result;
#endif
}

void VM::_creat() // 8
{
    std::string path = readstrp(getInc(7, 2));
    int mode = read16(getInc(7, 2));
    if (trace) debug("sys creat; \"" + path + "\"; 0" + oct(mode, 3));
    std::string path2 = convpath(path);
#if WIN32
    int result = open(path2.c_str(), O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0777);
#else
    int result = creat(path2.c_str(), mode);
#endif
    r[0] = (C = (result == -1)) ? errno : result;
    if (!C)
    {
        fd2name[result] = path2;
        handles.push_back(result);
    }
}

void VM::_link() // 9
{
    std::string src = readstrp(getInc(7, 2));
    std::string dst = readstrp(getInc(7, 2));
    if (trace) debug("sys link; \"" + src + "\"; \"" + dst + "\"");
#ifdef WIN32
    procResult(this, CopyFileA(src.c_str(), dst.c_str(), TRUE));
#else
    int result = link(convpath(src).c_str(), convpath(dst).c_str());
    r[0] = (C = (result == -1)) ? errno : result;
#endif
}

void VM::_unlink() // 10
{
    std::string path = readstrp(getInc(7, 2));
    if (trace) debug("sys unlink; \"" + path + "\"");
    std::string path2 = convpath(path);
#ifdef WIN32
    procResult(this, DeleteFileA(path2.c_str()));
    if (C)
    {
        struct stat st;
        if (stat(path2.c_str(), &st) != -1)
        {
            if (trace)
                fprintf(stderr, "register delayed: %s\n", path2.c_str());
            unlinks.push_back(path2);
        }
    }
#else
    int result = unlink(path2.c_str());
    r[0] = (C = (result == -1)) ? errno : result;
#endif
}

void VM::_exec() // 11
{
    std::string path = readstrp(getInc(7, 2));
    std::vector<std::string> args = getArgs(-1, read16(getInc(7, 2)));
    if (trace)
    {
        std::string buf = "sys exec; \"" + path + "\"; { ";
        for (int i = 0; i < (int)args.size(); i++)
        {
            if (i > 0) buf += ", ";
            buf += "\"" + args[i] + "\"";
        }
        debug(buf + " }");
    }
    AOut aout(convpath(path));
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
        int result = execv(convpath(path).c_str(), const_cast<char *const *>(&argv[0]));
        r[0] = (C = (result == -1)) ? errno : result;
    }
}

void VM::_chdir() // 12
{
    std::string path = readstrp(getInc(7, 2));
    if (trace) debug("sys chdir; \"" + path + "\"");
    int result = chdir(convpath(path).c_str());
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_chmod() // 15
{
    std::string path = readstrp(getInc(7, 2));
    int mode = read16(getInc(7, 2));
    if (trace) debug("sys chmod; \"" + path + "\"; 0" + oct(mode, 3));
    int result = chmod(convpath(path).c_str(), mode);
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_break() // 17
{
    int nd = read16(getInc(7, 2));
    if (trace) debug("sys break; " + hex(nd));
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
    std::string path = readstrp(getInc(7, 2));
    int p = read16(getInc(7, 2));
    if (trace) debug("sys stat; \"" + path + "\"; " + hex(p));
    struct stat st;
    int result = stat(convpath(path).c_str(), &st);
    C = result == -1;
    if (C)
        r[0] = errno;
    else
    {
        memset(&mem[p], 0, 36);
        write16(p     , st.st_dev);
        write16(p +  2, st.st_ino);
        write16(p +  4, st.st_mode);
        write8 (p +  6, st.st_nlink);
        write8 (p +  7, st.st_uid);
        write8 (p +  8, st.st_gid);
        write8 (p +  9, st.st_size >> 16);
        write16(p + 10, st.st_size);
        write16(p + 28, st.st_atime >> 16);
        write16(p + 30, st.st_atime);
        write16(p + 32, st.st_mtime >> 16);
        write16(p + 34, st.st_mtime);
        r[0] = result;
    }
}

void VM::_seek() // 19
{
    int fd = r[0];
    int p = read16(getInc(7, 2));
    int t = read16(getInc(7, 2));
    if (trace) debug("sys seek; " + hex(p) + "; " + str(t));
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
    if (trace) debug("sys getpid");
    int result = getpid();
    r[0] = (C = (result == -1)) ? errno : result;
}

void VM::_dup() // 41
{
    if (trace) debug("sys dup");
    int result = dup(r[0]);
    r[0] = (C = (result == -1)) ? errno : result;
}
