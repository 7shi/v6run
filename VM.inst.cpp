#include <stdlib.h>
#include "VM.h"

VM *VM::current;

void VM::runStep()
{
    current = this;
    if (nextPC)
    {
        r[7] = *nextPC;
        nextPC = NULL;
    }
    prevPC = r[7];
    if (prevPC & 1)
    {
        abort("invalid pc");
        return;
    }

    Operand src, dst;
    uint16_t srcv, dstv;
    uint8_t srcb, dstb;
    int val;
    switch (read8(r[7] + 1) >> 4)
    {
        case 0:
            exec0();
            return;
        case 1: // mov: MOVe
            getSrcDst(&src, &dst, 2);
            srcv = src.getValue(this);
            dst.setValue(this, srcv);
            setFlags(srcv == 0, int16_t(srcv) < 0, C, false);
            return;
        case 2: // cmp: CoMPare
            getSrcDst(&src, &dst, 2);
            srcv = src.getValue(this);
            dstv = dst.getValue(this);
            val = int(int16_t(srcv)) - int(int16_t(dstv));
            setFlags(val == 0, val < 0, srcv < dstv, val < -0x8000);
            return;
        case 3: // bit: BIt Test
            getSrcDst(&src, &dst, 2);
            val = src.getValue(this) & dst.getValue(this);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 4: // bic: BIt Clear
            getSrcDst(&src, &dst, 2);
            val = (~src.getValue(this)) & dst.getValue(this, true);
            dst.setValue(this, val);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 5: // bis: BIt Set
            getSrcDst(&src, &dst, 2);
            val = src.getValue(this) | dst.getValue(this, true);
            dst.setValue(this, val);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 6: // add: ADD
            getSrcDst(&src, &dst, 2);
            srcv = src.getValue(this);
            dstv = dst.getValue(this, true);
            val = int(int16_t(srcv)) + int(int16_t(dstv));
            dst.setValue(this, val);
            setFlags(val == 0, val < 0, int(srcv) + int(dstv) >= 0x10000, val >= 0x8000);
            return;
        case 7:
            exec7();
            return;
        case 010:
            exec10();
            return;
        case 011: // movb: MOVe Byte
            getSrcDst(&src, &dst, 1);
            srcb = src.getByte(this);
            dst.setByte(this, srcb);
            setFlags(srcb == 0, int8_t(srcb) < 0, C, false);
            return;
        case 012: // cmpb: CoMPare Byte
            getSrcDst(&src, &dst, 1);
            srcb = src.getByte(this);
            dstb = dst.getByte(this);
            val = int(int8_t(srcb)) - int(int8_t(dstb));
            setFlags(val == 0, val < 0, srcb < dstb, val < -0x80);
            return;
        case 013: // bitb: BIt Test Byte
            getSrcDst(&src, &dst, 1);
            val = src.getByte(this) & dst.getByte(this);
            setFlags(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 014: // bicb: BIt Clear Byte
            getSrcDst(&src, &dst, 1);
            val = (~src.getByte(this)) & dst.getByte(this, true);
            dst.setByte(this, val);
            setFlags(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 015: // bisb: BIt Set Byte
            getSrcDst(&src, &dst, 1);
            val = src.getByte(this) | dst.getByte(this, true);
            dst.setByte(this, val);
            setFlags(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 016: // sub: SUBtract
            getSrcDst(&src, &dst, 2);
            srcv = src.getValue(this);
            dstv = dst.getValue(this, true);
            val = int(int16_t(dstv)) - int(int16_t(srcv));
            dst.setValue(this, val);
            setFlags(val == 0, val < 0, dstv < srcv, val < -0x8000);
            return;
        case 017:
            exec17();
            return;
    }
    abort("not implemented");
}

void VM::exec0()
{
    switch (read8(r[7] + 1))
    {
        case 1: // br: BRanch
            r[7] = getOffset(r[7]);
            return;
        case 2: // bne: Branch if Not Equal
            r[7] = !Z ? getOffset(r[7]) : r[7] + 2;
            return;
        case 3: // beq: Branch if EQual
            r[7] = Z ? getOffset(r[7]) : r[7] + 2;
            return;
        case 4: // bge: Branch if Greater or Equal
            r[7] = !(N ^ V) ? getOffset(r[7]) : r[7] + 2;
            return;
        case 5: // blt: Branch if Less Than
            r[7] = N ^ V ? getOffset(r[7]) : r[7] + 2;
            return;
        case 6: // bgt: Branch if Greater Than
            r[7] = !(Z || (N ^V)) ? getOffset(r[7]) : r[7] + 2;
            return;
        case 7: // ble: Branch if Less or Equal
            r[7] = Z || (N ^ V) ? getOffset(r[7]) : r[7] + 2;
            return;
    }
    int v = read16(r[7]);
    if (v == 0240) // nop: No OPeration
    {
        r[7] += 2;
        return;
    }
    int v1 = (v >> 9) & 7, v2 = (v >> 6) & 7, reg, val;
    Operand dst;
    switch (v1)
    {
        case 0: // 00 0x xx
            switch (v2)
            {
                //case 0: // 00 00 xx
                //    switch (v & 077)
                //    {
                //        case 0: // halt
                //        case 1: // wait
                //        case 2: // rti
                //        case 3: // bpt
                //        case 4: // iot
                //        case 5: // reset
                //        case 6: // rtt
                //    }
                case 1: // jmp: JuMP
                    getDst(&dst, 2);
                    r[7] = dst.getAddress(this);
                    return;
                case 2: // 00 02 xx
                    switch ((v >> 3) & 7)
                    {
                        //case 3: // spl
                        case 0: // rts: ReTurn from Subroutine
                            reg = v & 7;
                            r[7] = r[reg];
                            r[reg] = read16(getInc(6, 2));
                            return;
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        {
                            // cl*/se*/ccc/scc: CLear/SEt (Condition Codes)
                            bool f = (v & 16) != 0;
                            if ((v & 8) != 0) N = f;
                            if ((v & 4) != 0) Z = f;
                            if ((v & 2) != 0) V = f;
                            if ((v & 1) != 0) C = f;
                            r[7] += 2;
                            return;
                        }
                    }
                case 3: // swab: SWAp Bytes
                {
                    getDst(&dst, 2);
                    uint16_t val0 = dst.getValue(this, true);
                    uint16_t bh = (val0 >> 8) & 0xff;
                    uint16_t bl = val0 & 0xff;
                    int val1 = (bl << 8) | bh;
                    dst.setValue(this, val1);
                    setFlags(val1 == 0, (val1 & 0x8000) != 0, false, false);
                    return;
                }
            }
        case 4: // jsr: Jump to SubRoutine
        {
            reg = (v >> 6) & 7;
            getDst(&dst, 2);
            int tmp = dst.getAddress(this);
            write16(getDec(6, 2), r[reg]);
            r[reg] = r[7];
            r[7] = tmp;
            return;
        }
        case 5:
            switch (v2)
            {
                case 0: // clr: CLeaR
                    getDst(&dst, 2);
                    dst.setValue(this, 0);
                    setFlags(true, false, false, false);
                    return;
                case 1: // com: COMplement
                    getDst(&dst, 2);
                    val = ~dst.getValue(this);
                    dst.setValue(this, val);
                    setFlags(val == 0, (val & 0x8000) != 0, true, false);
                    return;
                case 2: // inc: INCrement
                    getDst(&dst, 2);
                    val = int(int16_t(dst.getValue(this, true))) + 1;
                    dst.setValue(this, val);
                    setFlags(val == 0, val < 0, C, val == 0x8000);
                    return;
                case 3: // dec: DECrement
                    getDst(&dst, 2);
                    val = int(int16_t(dst.getValue(this, true))) - 1;
                    dst.setValue(this, val);
                    setFlags(val == 0, val < 0, C, val == -0x8001);
                    return;
                case 4: // neg: NEGate
                {
                    getDst(&dst, 2);
                    int val0 = dst.getValue(this, true);
                    int val1 = -int16_t(val0);
                    dst.setValue(this, val1);
                    setFlags(val1 == 0, val1 < 0, val1 != 0, val1 == 0x8000);
                    return;
                }
                case 5: // adc: ADd Carry
                    getDst(&dst, 2);
                    val = int(int16_t(dst.getValue(this, true))) + (C ? 1 : 0);
                    dst.setValue(this, val);
                    setFlags(val == 0, val < 0, C && val == 0, val == 0x8000);
                    return;
                case 6: // sbc: SuBtract Carry
                    getDst(&dst, 2);
                    val = int(int16_t(dst.getValue(this, true))) - (C ? 1 : 0);
                    dst.setValue(this, val);
                    setFlags(val == 0, val < 0, C && val == -1, val == -0x8001);
                    return;
                case 7: // tst: TeST
                    getDst(&dst, 2);
                    val = int16_t(dst.getValue(this));
                    setFlags(val == 0, val < 0, false, false);
                    return;
            }
        case 6:
            switch (v2)
            {
                //case 5: mfpi
                //case 6: mtpi
                case 0: // ror: ROtate Right
                {
                    getDst(&dst, 2);
                    int val0 = dst.getValue(this, true);
                    int val1 = (val0 >> 1) | (C ? 0x8000 : 0);
                    dst.setValue(this, val1);
                    bool lsb0 = (val0 & 1) != 0;
                    bool msb1 = C;
                    setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
                    return;
                }
                case 1: // rol: ROtate Left
                {
                    getDst(&dst, 2);
                    int val0 = dst.getValue(this, true);
                    int val1 = uint16_t(val0 << 1) | (C ? 1 : 0);
                    dst.setValue(this, val1);
                    bool msb0 = (val0 & 0x8000) != 0;
                    bool msb1 = (val1 & 0x8000) != 0;
                    setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
                    return;
                }
                case 2: // asr: Arithmetic Shift Right
                {
                    getDst(&dst, 2);
                    int val0 = dst.getValue(this, true);
                    int val1 = int16_t(val0) >> 1;
                    dst.setValue(this, val1);
                    bool lsb0 = (val0 & 1) != 0;
                    bool msb1 = val1 < 0;
                    setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
                    return;
                }
                case 3: // asl: Arithmetic Shift Left
                {
                    getDst(&dst, 2);
                    int val0 = dst.getValue(this);
                    int val1 = uint16_t((uint32_t(val0) << 1) & 0xffff);
                    dst.setValue(this, val1);
                    bool msb0 = (val0 & 0x8000) != 0;
                    bool msb1 = val1 < 0;
                    setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
                    return;
                }
                case 4: // mark: MARK
                {
                    int nn = v & 077;
                    r[6] = uint16_t((r[6] + 2 * nn) & 0xffff);
                    r[7] = r[5];
                    r[5] = read16(getInc(6, 2));
                    return;
                }
                case 7: // sxt: Sign eXTend
                {
                    getDst(&dst, 2);
                    dst.setValue(this, N ? 0xffff : 0);
                    setFlags(!N, N, C, V);
                    return;
                }
            }
    }
    abort("not implemented");
}

void VM::exec7()
{
    int v = read16(r[7]), reg, val;
    Operand dst;
    switch ((v >> 9) & 7)
    {
        //case 5:
        //    switch ((v >> 3) & 077
        //    {
        //        case 0: fadd
        //        case 1: fsub
        //        case 2: fmul
        //        case 3: fdiv
        //    }
        case 0: // mul:MULtiply
        {
            getDst(&dst, 2);
            int src = int16_t(dst.getValue(this));
            reg = (v >> 6) & 7;
            val = int(r[reg]) * src;
            if ((reg & 1) == 0)
                setReg32(reg, val);
            else
                r[reg] = val;
            setFlags(val == 0, val < 0, val < -0x8000 || val >= 0x8000, false);
            return;
        }
        case 1: // div: DIVide
        {
            getDst(&dst, 2);
            int src = int16_t(dst.getValue(this));
            reg = (v >> 6) & 7;
            if (src == 0 || abs(int16_t(r[reg])) > abs(src))
                setFlags(false, false, src == 0, true);
            else
            {
                val = getReg32(reg);
                int r1 = val / src;
                r[reg] = r1;
                r[(reg + 1) & 7] = val % src;
                setFlags(r1 == 0, r1 < 0, false, false);
            }
            return;
        }
        case 2: // ash: Arithmetic SHift
        {
            getDst(&dst, 2);
            int src = dst.getValue(this) & 077;
            reg = (v >> 6) & 7;
            int val0 = int16_t(r[reg]);
            if (src == 0)
                setFlags(val0 == 0, val0 < 0, C, false);
            else
            {
                if ((src & 040) == 0)
                {
                    int val1 = val0 << (src - 1);
                    int val2 = val1 << 1;
                    r[reg] = val2;
                    setFlags(val2 == 0, val2 < 0, (val1 & 1) != 0, val0 != val2);
                }
                else
                {
                    int val1 = val0 >> (63 - src);
                    int val2 = val1 >> 1;
                    r[reg] = val2;
                    setFlags(val2 == 0, val2 < 0, val1 < 0, val0 != val2);
                }
            }
            return;
        }
        case 3: // ashc: Arithmetic SHift Combined
        {
            getDst(&dst, 2);
            int src = dst.getValue(this) & 077;
            reg = (v >> 6) & 7;
            int val0 = getReg32(reg);
            if (src == 0)
                setFlags(val0 == 0, val0 < 0, C, false);
            else
            {
                if ((src & 040) == 0)
                {
                    int val1 = val0 << (src - 1);
                    int val2 = val1 << 1;
                    setReg32(reg, val2);
                    setFlags(val2 == 0, val2 < 0, (val1 & 1) != 0, val0 != val2);
                }
                else
                {
                    int val1 = val0 >> (63 - src);
                    int val2 = val1 >> 1;
                    setReg32(reg, val2);
                    setFlags(val2 == 0, val2 < 0, val1 < 0, val0 != val2);
                }
            }
            return;
        }
        case 4: // xor: eXclusive OR
            getDst(&dst, 2);
            val = r[(v >> 6) & 7] ^ dst.getValue(this, true);
            dst.setValue(this, val);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 7: // sob: Subtract One from register, Branch if not zero
            reg = (v >> 6) & 7;
            r[reg]--;
            r[7] = r[reg] != 0 ? r[7] + 2 - (v & 077) * 2 : r[7] + 2;
            return;
    }
    abort("not implemented");
}

void VM::exec10()
{
    switch (read8(r[7] + 1))
    {
        //case 0x88: emt
        case 0x80: // bpl: Branch if PLus
            r[7] = !N ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x81: // bmi: Branch if MInus
            r[7] = N ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x82: // bhi: Branch if HIgher
            r[7] = !(C | Z) ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x83: // blos: Branch if LOwer or Same
            r[7] = C | Z ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x84: // bvc: Branch if oVerflow Clear
            r[7] = !V ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x85: // bvs: Branch if oVerflow Set
            r[7] = V ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x86: // bcc: Branch if Carry Clear
            r[7] = !C ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x87: // bcs: Branch if Carry Set
            r[7] = C ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x89: // sys
            sys();
            return;
    }
    int v = read16(r[7]);
    Operand dst;
    int val;
    switch ((v >> 6) & 077)
    {
        //case 064: mfpd
        //case 065: mtpd
        case 050: // clrb: CLeaR Byte
            getDst(&dst, 1);
            dst.setByte(this, 0);
            setFlags(true, false, false, false);
            return;
        case 051: // comb: COMplement Byte
            getDst(&dst, 1);
            val = ~dst.getByte(this, true);
            dst.setByte(this, val);
            setFlags(val == 0, (val & 0x80) != 0, true, false);
            return;
        case 052: // incb: INCrement Byte
            getDst(&dst, 1);
            val = int(int8_t(dst.getByte(this, true))) + 1;
            dst.setByte(this, val);
            setFlags(val == 0, val < 0, C, val == 0x80);
            return;
        case 053: // decb: DECrement Byte
            getDst(&dst, 1);
            val = int(int8_t(dst.getByte(this, true))) - 1;
            dst.setByte(this, val);
            setFlags(val == 0, val < 0, C, val == -0x81);
            return;
        case 054: // negb: NEGate Byte
        {
            getDst(&dst, 1);
            int val0 = dst.getByte(this, true);
            int val1 = -int8_t(val0);
            dst.setByte(this, val1);
            setFlags(val1 == 0, val1 < 0, val1 != 0, val1 == 0x80);
            return;
        }
        case 055: // adcb: ADd Carry Byte
            getDst(&dst, 1);
            val = int(int8_t(dst.getByte(this, true))) + (C ? 1 : 0);
            dst.setByte(this, val);
            setFlags(val == 0, val < 0, C && val == 0, val == 0x80);
            return;
        case 056: // sbcb: SuBtract Carry Byte
            getDst(&dst, 1);
            val = int(int8_t(dst.getByte(this, true))) - (C ? 1 : 0);
            dst.setByte(this, val);
            setFlags(val == 0, val < 0, C && val == -1, val == -0x81);
            return;
        case 057: // tstb: TeST Byte
            getDst(&dst, 1);
            val = int8_t(dst.getByte(this));
            setFlags(val == 0, val < 0, false, false);
            return;
        case 060: // rorb: ROtate Right Byte
        {
            getDst(&dst, 1);
            int val0 = dst.getByte(this, true);
            int val1 = val0 >> 1;
            if (C) val1 = uint8_t(val1 + 0x80);
            dst.setByte(this, val1);
            bool lsb0 = (val0 & 1) != 0;
            bool msb1 = C;
            setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
            return;
        }
        case 061: // rolb: ROtate Left Byte
        {
            getDst(&dst, 1);
            int val0 = dst.getByte(this, true);
            int val1 = uint8_t(((uint32_t(val0) << 1) + (C ? 1 : 0)) & 0xff);
            dst.setByte(this, val1);
            bool msb0 = (val0 & 0x80) != 0;
            bool msb1 = (val1 & 0x80) != 0;
            setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
            return;
        }
        case 062: // asrb: Arithmetic Shift Right Byte
        {
            getDst(&dst, 1);
            int val0 = dst.getByte(this, true);
            int val1 = int8_t(val0) >> 1;
            dst.setByte(this, val1);
            bool lsb0 = (val0 & 1) != 0;
            bool msb1 = val1 < 0;
            setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
            return;
        }
        case 063: // aslb: Arithmetic Shift Left Byte
        {
            getDst(&dst, 1);
            int val0 = dst.getByte(this, true);
            int val1 = uint8_t((uint32_t(val0) << 1) & 0xff);
            dst.setByte(this, val1);
            bool msb0 = (val0 & 0x80) != 0;
            bool msb1 = val1 < 0;
            setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
            return;
        }
    }
    abort("not implemented");
}

void VM::exec17()
{
    int v = read16(r[7]);
    switch (v & 0xfff)
    {
        case 1: // setf: SET Float
            r[7] += 2;
            isDouble = false;
            return;
        case 2: // seti: SET Integer
            r[7] += 2;
            isLong = false;
            return;
        case 011: // setd: SET Double
            r[7] += 2;
            isDouble = true;
            return;
        case 012: // setl: SET Long
            r[7] += 2;
            isLong = true;
            return;
    }
    abort("not implemented");
}
