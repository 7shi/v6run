#include <stdio.h>
#include <sys/stat.h>
#include "pdp11.h"

AOut::AOut(const std::string &path)
{
    this->path = path;

    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 16)
        return;

    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return;

    std::vector<uint8_t> header(16);
    fread(&header[0], 1, 16, f);
    fmagic = read16(header,  0);
    tsize  = read16(header,  2);
    dsize  = read16(header,  4);
    bsize  = read16(header,  6);
    ssize  = read16(header,  8);
    entry  = read16(header, 10);
    pad    = read16(header, 12);
    relflg = read16(header, 14);

    if (st.st_size - 16 >= tsize + dsize)
    {
        image.resize(tsize + dsize);
        fread(&image[0], 1, image.size(), f);
    }

    fclose(f);
}
