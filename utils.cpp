#include "utils.h"

std::string hex(int v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%04x", v & 0xffff);
    return std::string(buf);
}
