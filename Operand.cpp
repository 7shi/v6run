#include "Operand.h"
#include "VM.h"

void Operand::set(int t, int r, VM *vm, int ad, int sz)
{
    type = t;
    reg = r;
    dist = length = 0;
    pc = ad;
    size = sz;
    if (t >= 6)
    {
        dist = vm->read16(ad);
        length = 2;
        pc += 2;
    }
    else if (r == 7 && (t == 2 || t == 3))
        length = 2;
}

uint16_t Operand::getAddress(VM *vm)
{
    if (reg == 7)
    {
        switch (type)
        {
            case 2: return pc;
            case 3: return vm->read16(pc);
            case 6: return pc + dist;
            case 7: return vm->read16(pc + dist);
        }
    }
    else
    {
        switch (type)
        {
            case 2: return vm->getInc(reg, 2);
            case 3: return vm->read16(vm->getInc(reg, 2));
            case 4: return vm->getDec(reg, 2);
            case 5: return vm->read16(vm->getDec(reg, 2));
            case 6: return vm->r[reg] + dist;
            case 7: return vm->read16(vm->r[reg] + dist);
        }
    }
    vm->abort("invalid operand");
    return 0;
}

uint16_t Operand::getValue(VM *vm, bool nomove)
{
    if (reg == 7)
    {
        switch (type)
        {
            case 0: return pc;
            case 1:
            case 2: return vm->read16(pc);
            case 3: return vm->read16(vm->read16(pc));
            case 6: return vm->read16(pc + dist);
            case 7: return vm->read16(vm->read16(pc + dist));
        }
    }
    else if (nomove)
    {
        switch (type)
        {
            case 0: return vm->r[reg];
            case 1:
            case 2: return vm->read16(vm->r[reg]);
            case 3: return vm->read16(vm->read16(vm->r[reg]));
            case 4: return vm->read16(vm->r[reg] - 2);
            case 5: return vm->read16(vm->read16(vm->r[reg] - 2));
            case 6: return vm->read16(vm->r[reg] + dist);
            case 7: return vm->read16(vm->read16(vm->r[reg] + dist));
        }
    }
    else
    {
        switch (type)
        {
            case 0: return vm->r[reg];
            case 1: return vm->read16(vm->r[reg]);
            case 2: return vm->read16(vm->getInc(reg, 2));
            case 3: return vm->read16(vm->read16(vm->getInc(reg, 2)));
            case 4: return vm->read16(vm->getDec(reg, 2));
            case 5: return vm->read16(vm->read16(vm->getDec(reg, 2)));
            case 6: return vm->read16(vm->r[reg] + dist);
            case 7: return vm->read16(vm->read16(vm->r[reg] + dist));
        }
    }
    vm->abort("invalid operand");
    return 0;
}

void Operand::setValue(VM *vm, uint16_t v)
{
    if (reg == 7)
    {
        switch (type)
        {
            case 0: pc = v; return;
            case 1:
            case 2: vm->write16(pc, v); return;
            case 3: vm->write16(vm->read16(pc), v); return;
            case 6: vm->write16(pc + dist, v); return;
            case 7: vm->write16(vm->read16(pc + dist), v); return;
        }
    }
    else
    {
        switch (type)
        {
            case 0: vm->r[reg] = v; return;
            case 1: vm->write16(vm->r[reg], v); return;
            case 2: vm->write16(vm->getInc(reg, 2), v); return;
            case 3: vm->write16(vm->read16(vm->getInc(reg, 2)), v); return;
            case 4: vm->write16(vm->getDec(reg, 2), v); return;
            case 5: vm->write16(vm->read16(vm->getDec(reg, 2)), v); return;
            case 6: vm->write16(vm->r[reg] + dist, v); return;
            case 7: vm->write16(vm->read16(vm->r[reg] + dist), v); return;
        }
    }
    vm->abort("invalid operand");
}

uint8_t Operand::getByte(VM *vm, bool nomove)
{
    int size = reg == 6 ? 2 : 1;
    if (reg == 7)
    {
        switch (type)
        {
            case 0: return pc;
            case 1:
            case 2: return vm->read8(pc);
            case 3: return vm->read8(vm->read16(pc));
            case 6: return vm->read8(pc + dist);
            case 7: return vm->read8(vm->read16(pc + dist));
        }
    }
    else if (nomove)
    {
        switch (type)
        {
            case 0: return vm->r[reg];
            case 1:
            case 2: return vm->read8(vm->r[reg]);
            case 3: return vm->read8(vm->read16(vm->r[reg]));
            case 4: return vm->read8(vm->r[reg] - size);
            case 5: return vm->read8(vm->read16(vm->r[reg] - 2));
            case 6: return vm->read8(vm->r[reg] + dist);
            case 7: return vm->read8(vm->read16(vm->r[reg] + dist));
        }
    }
    else
    {
        switch (type)
        {
            case 0: return vm->r[reg];
            case 1: return vm->read8(vm->r[reg]);
            case 2: return vm->read8(vm->getInc(reg, size));
            case 3: return vm->read8(vm->read16(vm->getInc(reg, 2)));
            case 4: return vm->read8(vm->getDec(reg, size));
            case 5: return vm->read8(vm->read16(vm->getDec(reg, 2)));
            case 6: return vm->read8(vm->r[reg] + dist);
            case 7: return vm->read8(vm->read16(vm->r[reg] + dist));
        }
    }
    vm->abort("invalid operand");
    return 0;
}

void Operand::setByte(VM *vm, uint8_t v)
{
    int size = reg == 6 ? 2 : 1;
    if (reg == 7)
    {
        switch (type)
        {
            case 0: pc = v < 0x80 ? v : v - 0x100; return;
            case 1:
            case 2: vm->write8(pc, v); return;
            case 3: vm->write8(vm->read16(pc), v); return;
            case 6: vm->write8(pc + dist, v); return;
            case 7: vm->write8(vm->read16(pc + dist), v); return;
        }
    }
    else
    {
        switch (type)
        {
            case 0: vm->r[reg] = v < 0x80 ? v : v - 0x100; return;
            case 1: vm->write8(vm->r[reg], v); return;
            case 2: vm->write8(vm->getInc(reg, size), v); return;
            case 3: vm->write8(vm->read16(vm->getInc(reg, 2)), v); return;
            case 4: vm->write8(vm->getDec(reg, size), v); return;
            case 5: vm->write8(vm->read16(vm->getDec(reg, 2)), v); return;
            case 6: vm->write8(vm->r[reg] + dist, v); return;
            case 7: vm->write8(vm->read16(vm->r[reg] + dist), v); return;
        }
    }
    vm->abort("invalid operand");
}
