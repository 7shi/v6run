#include "VM.h"

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
      //case 12: _chdir (); return;
      //case 13: _time  (); return;
      //case 14: _mknod (); return;
        case 15: _chmod (); return;
      //case 16: _chown (); return;
        case 17: _break (); return;
        case 18: _stat  (); return;
        case 19: _seek  (); return;
        case 20: _getpid(); return;
      //case 21: _mount (); return;
      //case 22: _umount(); return;
      //case 23: _setuid(); return;
      //case 24: _getuid(); return;
      //case 25: _stime (); return;
      //case 26: _ptrace(); return;
      //case 28: _fstat (); return;
      //case 30: _smdate(); return;
      //case 31: _stty  (); return;
      //case 32: _gtty  (); return;
      //case 34: _nice  (); return;
      //case 35: _sleep (); return;
      //case 36: _sync  (); return;
      //case 37: _kill  (); return;
      //case 38: _switch(); return;
        case 41: _dup   (); return;
      //case 42: _pipe  (); return;
      //case 43: _times (); return;
      //case 44: _prof  (); return;
      //case 45: _tiu   (); return;
      //case 46: _setgid(); return;
      //case 47: _getgid(); return;
        case 48: _signal(); return;
    }
    abort("invalid sys");
}

void VM::_indir() // 0
{
}

void VM::_exit() // 1
{
}

void VM::_fork() // 2
{
}

void VM::_read() // 3
{
}

void VM::_write() // 4
{
}

void VM::_open() // 5
{
}

void VM::_close() // 6
{
}

void VM::_wait() // 7
{
}

void VM::_creat() // 8
{
}

void VM::_link() // 9
{
}

void VM::_unlink() // 10
{
}

void VM::_exec() // 11
{
}

//void VM::_chdir(); // 12
//void VM::_time (); // 13
//void VM::_mknod(); // 14

void VM::_chmod() // 15
{
}

//void VM::_chown(); // 16

void VM::_break() // 17
{
}

void VM::_stat() // 18
{
}

void VM::_seek() // 19
{
}

void VM::_getpid() // 20
{
}

//void VM::_mount (); // 21
//void VM::_umount(); // 22
//void VM::_setuid(); // 23
//void VM::_getuid(); // 24
//void VM::_stime (); // 25
//void VM::_ptrace(); // 26
//void VM::_fstat (); // 28
//void VM::_smdate(); // 30
//void VM::_stty  (); // 31
//void VM::_gtty  (); // 32
//void VM::_nice  (); // 34
//void VM::_sleep (); // 35
//void VM::_sync  (); // 36
//void VM::_kill  (); // 37
//void VM::_switch(); // 38

void VM::_dup() // 41
{
}

//void VM::_pipe  (); // 42
//void VM::_times (); // 43
//void VM::_prof  (); // 44
//void VM::_tiu   (); // 45
//void VM::_setgid(); // 46
//void VM::_getgid(); // 47

void VM::_signal() // 48
{
}
