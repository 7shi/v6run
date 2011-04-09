#include <stdio.h>
#include "VM.h"

int main(int argc, char *argv[])
{
    bool verbose = false;
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
            verbose = true;
        else
            args.push_back(arg);
    }
    if (args.empty())
    {
        printf("usage: %s [-r V6ROOT] [-v] cmd [args ...]\n", argv[0]);
        return 1;
    }
    AOut aout = convpath(args[0]);
    if (aout.image.empty())
    {
        printf("%s\n", aout.error.c_str());
        return 0;
    }
    VM vm = &aout;
    vm.verbose = verbose;
    vm.run(args);
}
