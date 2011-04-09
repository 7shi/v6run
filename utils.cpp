#include <sys/stat.h>
#include "utils.h"

static std::string rootpath;

std::string hex(int v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%04x", v & 0xffff);
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
