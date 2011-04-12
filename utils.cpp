#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#ifdef WIN32
#include <windows.h>
#endif
#include "utils.h"

std::string regnames[] = { "r0", "r1", "r2", "r3", "r4", "r5", "sp", "pc" };

static std::string rootpath;

std::string hex(int v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%04x", v & 0xffff);
    return std::string(buf);
}

std::string oct(int v, int len)
{
    std::string ret;
    char buf[] = { 0, 0 };
    while (v)
    {
        buf[0] = '0' + (v & 7);
        ret = buf + ret;
        v >>= 3;
    }
    if (ret.empty()) ret = "0";
    while (len > 0 && (int)ret.size() < len)
        ret = "0" + ret;
    return ret;
}

std::string str(int v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", v);
    return std::string(buf);
}

std::string sdist(int v)
{
    char buf[32];
    int d = int16_t(uint16_t(v));
    int a = abs(d);
    if (a < 10)
        snprintf(buf, sizeof(buf), "%d", d);
    else
        snprintf(buf, sizeof(buf), "%s%x", d < 0 ? "-" : "", a);
    return std::string(buf);
}

void setroot(std::string root)
{
    while (!root.empty() && root[root.size() - 1] == '/')
        root = root.substr(0, root.size() - 1);
    rootpath = root;
}

std::string convpath(const std::string &path)
{
    if (path.empty() || path[0] != '/' || rootpath.empty()) return path;

#ifdef WIN32
    if (startsWith(path, "/tmp/"))
        return getenv("TEMP") + path.substr(4);
#endif
    std::string path2 = rootpath + path;
    struct stat st;
    if (stat(path2.c_str(), &st) == 0) return path2;

    return path;
}

bool startsWith(const std::string &s, const std::string &prefix)
{
    if (s.size() < prefix.size()) return false;
    return s.substr(0, prefix.size()) == prefix;
}

bool endsWith(const std::string &s, const std::string &prefix)
{
    if (s.size() < prefix.size()) return false;
    return s.substr(s.size() - prefix.size(), prefix.size()) == prefix;
}

std::string replace(const std::string &src, const std::string &s1, const std::string &s2)
{
    if (s1.empty()) return src;
    std::string ret;
    int p = 0;
    while (p < (int)src.size())
    {
        int pp = src.find(s1, p);
        if (pp < 0)
        {
            ret += src.substr(p);
            break;
        }
        ret += src.substr(p, pp - p) + s2;
        p = pp + s1.size();
    }
    return ret;
}

#ifdef WIN32
std::string getErrorMessage(int err)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );
    std::string ret = replace((const char *)lpMsgBuf, "\r\n", "\n");
    LocalFree(lpMsgBuf);
    if (!endsWith(ret, "\n")) ret += "\n";
    return ret;
}
#endif
