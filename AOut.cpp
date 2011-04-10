#include <cstdio>
#include <sys/stat.h>
#include "AOut.h"
#include "utils.h"

AOut::AOut(const std::string &path)
{
    set(path);
}

void AOut::set(const std::string &path)
{
    this->path = path;
    image.clear();
    error.clear();

    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 16)
    {
        error = path + ": file not found";
        return;
    }

    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
    {
        error = path + ": can not open";
        return;
    }

    std::vector<uint8_t> header(16);
    fread(&header[0], 1, 16, f);
    fmagic = readvec16(header,  0);
    if (fmagic != 0407 && fmagic != 0410)
    {
        error = path + ": not PDP-11 a.out file";
        return;
    }

    tsize  = readvec16(header,  2);
    dsize  = readvec16(header,  4);
    bsize  = readvec16(header,  6);
    ssize  = readvec16(header,  8);
    entry  = readvec16(header, 10);
    pad    = readvec16(header, 12);
    relflg = readvec16(header, 14);

    if (st.st_size - 16 >= tsize + dsize)
    {
        image.resize(tsize + dsize);
        fread(&image[0], 1, image.size(), f);
    }

    fclose(f);
}
