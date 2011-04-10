#include <csignal>
#include <cerrno>
#include "VM.h"

#define V6_NSIG    20
#define V6_SIGHUP   1
#define V6_SIGINT   2
#define V6_SIGQIT   3
#define V6_SIGINS   4
#define V6_SIGTRC   5
#define V6_SIGIOT   6
#define V6_SIGEMT   7
#define V6_SIGFPT   8
#define V6_SIGKIL   9
#define V6_SIGBUS  10
#define V6_SIGSEG  11
#define V6_SIGSYS  12
#define V6_SIGPIPE 13

static uint16_t handlers[V6_NSIG];

void VM::sighandler(int sig)
{
    current->nextPC = &handlers[sig];
}

void VM::_signal() // 48
{
    int sig = read16(getInc(7, 2));
    int pcb = read16(getInc(7, 2));
    if (trace) debug("sys signal; " + str(sig) + "; " + hex(pcb));
    if (sig < V6_NSIG)
    {
        r[0] = handlers[sig];
        handlers[sig] = pcb;
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        if (!(pcb & 1)) sa.sa_handler = &sighandler;
        switch (sig)
        {
            case V6_SIGHUP : sigaction(SIGHUP , &sa, NULL); break;
            case V6_SIGINT : sigaction(SIGINT , &sa, NULL); break;
            case V6_SIGQIT : sigaction(SIGQUIT, &sa, NULL); break;
            case V6_SIGINS : sigaction(SIGILL , &sa, NULL); break;
            case V6_SIGTRC : sigaction(SIGTRAP, &sa, NULL); break;
            case V6_SIGIOT : sigaction(SIGIO  , &sa, NULL); break;
            //case V6_SIGEMT : sigaction(SIGEMT , &sa, NULL); break;
            case V6_SIGFPT : sigaction(SIGFPE , &sa, NULL); break;
            case V6_SIGKIL : sigaction(SIGKILL, &sa, NULL); break;
            case V6_SIGBUS : sigaction(SIGBUS , &sa, NULL); break;
            case V6_SIGSEG : sigaction(SIGSEGV, &sa, NULL); break;
            case V6_SIGSYS : sigaction(SIGSYS , &sa, NULL); break;
            case V6_SIGPIPE: sigaction(SIGPIPE, &sa, NULL); break;
        }
        C = false;
    }
    else
        C = true;
}
