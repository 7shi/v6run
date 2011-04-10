#include <stdio.h>
#include "VM.h"

int main(int argc, char *argv[])
{
    int trace = 0;
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-r")
        {
            i++;
            if (i < argc) setroot(argv[i]);
        }
        else if (arg == "-v")
            trace = 2;
        else if (arg == "-s" && trace == 0)
            trace = 1;
        else
            for (; i < argc; i++)
                args.push_back(argv[i]);
    }
    if (args.empty())
    {
        printf("usage: %s [-r V6ROOT] [-v/-s] cmd [args ...]\n", argv[0]);
        printf("    -v: verbose mode (output syscall and disassemble)\n");
        printf("    -s: syscall mode (output syscall)\n");
        return 1;
    }
    AOut aout = convpath(args[0]);
    if (aout.image.empty())
    {
        printf("%s\n", aout.error.c_str());
        return 0;
    }
    VM vm = &aout;
    vm.trace = trace;
    vm.run(args);
}
