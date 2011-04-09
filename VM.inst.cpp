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
            if (trace > 1) debug("mov", src, dst);
            srcv = src.getValue();
            dst.setValue(srcv);
            setFlags(srcv == 0, int16_t(srcv) < 0, C, false);
            return;
        case 2: // cmp: CoMPare
            getSrcDst(&src, &dst, 2);
            if (trace > 1) debug("cmp", src, dst);
            srcv = src.getValue();
            dstv = dst.getValue();
            val = int(int16_t(srcv)) - int(int16_t(dstv));
            setFlags(val == 0, val < 0, srcv < dstv, val < -0x8000);
            return;
        case 3: // bit: BIt Test
            getSrcDst(&src, &dst, 2);
            if (trace > 1) debug("bit", src, dst);
            val = src.getValue() & dst.getValue();
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 4: // bic: BIt Clear
            getSrcDst(&src, &dst, 2);
            if (trace > 1) debug("bic", src, dst);
            val = (~src.getValue()) & dst.getValue(true);
            dst.setValue(val);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 5: // bis: BIt Set
            getSrcDst(&src, &dst, 2);
            if (trace > 1) debug("bis", src, dst);
            val = src.getValue() | dst.getValue(true);
            dst.setValue(val);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 6: // add: ADD
            getSrcDst(&src, &dst, 2);
            if (trace > 1) debug("add", src, dst);
            srcv = src.getValue();
            dstv = dst.getValue(true);
            val = int(int16_t(srcv)) + int(int16_t(dstv));
            dst.setValue(val);
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
            if (trace > 1) debug("movb", src, dst);
            srcb = src.getByte();
            dst.setByte(srcb);
            setFlags(srcb == 0, int8_t(srcb) < 0, C, false);
            return;
        case 012: // cmpb: CoMPare Byte
            getSrcDst(&src, &dst, 1);
            if (trace > 1) debug("cmpb", src, dst);
            srcb = src.getByte();
            dstb = dst.getByte();
            val = int(int8_t(srcb)) - int(int8_t(dstb));
            setFlags(val == 0, val < 0, srcb < dstb, val < -0x80);
            return;
        case 013: // bitb: BIt Test Byte
            getSrcDst(&src, &dst, 1);
            if (trace > 1) debug("bitb", src, dst);
            val = src.getByte() & dst.getByte();
            setFlags(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 014: // bicb: BIt Clear Byte
            getSrcDst(&src, &dst, 1);
            if (trace > 1) debug("bicb", src, dst);
            val = (~src.getByte()) & dst.getByte(true);
            dst.setByte(val);
            setFlags(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 015: // bisb: BIt Set Byte
            getSrcDst(&src, &dst, 1);
            if (trace > 1) debug("bisb", src, dst);
            val = src.getByte() | dst.getByte(true);
            dst.setByte(val);
            setFlags(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 016: // sub: SUBtract
            getSrcDst(&src, &dst, 2);
            if (trace > 1) debug("sub", src, dst);
            srcv = src.getValue();
            dstv = dst.getValue(true);
            val = int(int16_t(dstv)) - int(int16_t(srcv));
            dst.setValue(val);
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
            if (trace > 1) debug("br");
            r[7] = getOffset(r[7]);
            
            return;
        case 2: // bne: Branch if Not Equal
            if (trace > 1) debug("bne");
            r[7] = !Z ? getOffset(r[7]) : r[7] + 2;
            return;
        case 3: // beq: Branch if EQual
            if (trace > 1) debug("beq");
            r[7] = Z ? getOffset(r[7]) : r[7] + 2;
            return;
        case 4: // bge: Branch if Greater or Equal
            if (trace > 1) debug("bge");
            r[7] = !(N ^ V) ? getOffset(r[7]) : r[7] + 2;
            return;
        case 5: // blt: Branch if Less Than
            if (trace > 1) debug("blt");
            r[7] = N ^ V ? getOffset(r[7]) : r[7] + 2;
            return;
        case 6: // bgt: Branch if Greater Than
            if (trace > 1) debug("bgt");
            r[7] = !(Z || (N ^V)) ? getOffset(r[7]) : r[7] + 2;
            return;
        case 7: // ble: Branch if Less or Equal
            if (trace > 1) debug("ble");
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
                    if (trace > 1) debug("jmp", dst);
                    r[7] = dst.getAddress();
                    return;
                case 2: // 00 02 xx
                    switch ((v >> 3) & 7)
                    {
                        //case 3: // spl
                        case 0: // rts: ReTurn from Subroutine
                            reg = v & 7;
                            if (trace > 1) debug("rts", reg);
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
                            if (trace > 1)
                            {
                                if (v == 0257)
                                    debug("ccc");
                                else if (v == 0277)
                                    debug("scc");
                                else
                                    debug(std::string((f ? "se" : "cl")) +
                                        (v & 8 ? "n" : "") +
                                        (v & 4 ? "z" : "") +
                                        (v & 2 ? "v" : "") +
                                        (v & 1 ? "c" : ""));
                            }
                            if (v & 8) N = f;
                            if (v & 4) Z = f;
                            if (v & 2) V = f;
                            if (v & 1) C = f;
                            r[7] += 2;
                            return;
                        }
                    }
                case 3: // swab: SWAp Bytes
                {
                    getDst(&dst, 2);
                    if (trace > 1) debug("swab", dst);
                    uint16_t val0 = dst.getValue(true);
                    uint16_t bh = (val0 >> 8) & 0xff;
                    uint16_t bl = val0 & 0xff;
                    int val1 = (bl << 8) | bh;
                    dst.setValue(val1);
                    setFlags(val1 == 0, (val1 & 0x8000) != 0, false, false);
                    return;
                }
            }
        case 4: // jsr: Jump to SubRoutine
        {
            reg = (v >> 6) & 7;
            getDst(&dst, 2);
            if (trace > 1) debug("jsr", reg, dst);
            int tmp = dst.getAddress();
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
                    if (trace > 1) debug("clr", dst);
                    dst.setValue(0);
                    setFlags(true, false, false, false);
                    return;
                case 1: // com: COMplement
                    getDst(&dst, 2);
                    if (trace > 1) debug("com", dst);
                    val = ~dst.getValue(true);
                    dst.setValue(val);
                    setFlags(val == 0, (val & 0x8000) != 0, true, false);
                    return;
                case 2: // inc: INCrement
                    getDst(&dst, 2);
                    if (trace > 1) debug("inc", dst);
                    val = int(int16_t(dst.getValue(true))) + 1;
                    dst.setValue(val);
                    setFlags(val == 0, val < 0, C, val == 0x8000);
                    return;
                case 3: // dec: DECrement
                    getDst(&dst, 2);
                    if (trace > 1) debug("dec", dst);
                    val = int(int16_t(dst.getValue(true))) - 1;
                    dst.setValue(val);
                    setFlags(val == 0, val < 0, C, val == -0x8001);
                    return;
                case 4: // neg: NEGate
                {
                    getDst(&dst, 2);
                    if (trace > 1) debug("neg", dst);
                    int val0 = dst.getValue(true);
                    int val1 = -int16_t(val0);
                    dst.setValue(val1);
                    setFlags(val1 == 0, val1 < 0, val1 != 0, val1 == 0x8000);
                    return;
                }
                case 5: // adc: ADd Carry
                    getDst(&dst, 2);
                    if (trace > 1) debug("adc", dst);
                    val = int(int16_t(dst.getValue(true))) + (C ? 1 : 0);
                    dst.setValue(val);
                    setFlags(val == 0, val < 0, C && val == 0, val == 0x8000);
                    return;
                case 6: // sbc: SuBtract Carry
                    getDst(&dst, 2);
                    if (trace > 1) debug("sbc", dst);
                    val = int(int16_t(dst.getValue(true))) - (C ? 1 : 0);
                    dst.setValue(val);
                    setFlags(val == 0, val < 0, C && val == -1, val == -0x8001);
                    return;
                case 7: // tst: TeST
                    getDst(&dst, 2);
                    if (trace > 1) debug("tst", dst);
                    val = int16_t(dst.getValue());
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
                    if (trace > 1) debug("ror", dst);
                    int val0 = dst.getValue(true);
                    int val1 = (val0 >> 1) | (C ? 0x8000 : 0);
                    dst.setValue(val1);
                    bool lsb0 = (val0 & 1) != 0;
                    bool msb1 = C;
                    setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
                    return;
                }
                case 1: // rol: ROtate Left
                {
                    getDst(&dst, 2);
                    if (trace > 1) debug("rol", dst);
                    int val0 = dst.getValue(true);
                    int val1 = uint16_t(val0 << 1) | (C ? 1 : 0);
                    dst.setValue(val1);
                    bool msb0 = (val0 & 0x8000) != 0;
                    bool msb1 = (val1 & 0x8000) != 0;
                    setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
                    return;
                }
                case 2: // asr: Arithmetic Shift Right
                {
                    getDst(&dst, 2);
                    if (trace > 1) debug("asr", dst);
                    int val0 = dst.getValue(true);
                    int val1 = int16_t(val0) >> 1;
                    dst.setValue(val1);
                    bool lsb0 = (val0 & 1) != 0;
                    bool msb1 = val1 < 0;
                    setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
                    return;
                }
                case 3: // asl: Arithmetic Shift Left
                {
                    getDst(&dst, 2);
                    if (trace > 1) debug("asl", dst);
                    int val0 = dst.getValue(true);
                    int val1 = uint16_t((uint32_t(val0) << 1) & 0xffff);
                    dst.setValue(val1);
                    bool msb0 = (val0 & 0x8000) != 0;
                    bool msb1 = val1 < 0;
                    setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
                    return;
                }
                case 4: // mark: MARK
                {
                    if (trace > 1) debug("mark");
                    int nn = v & 077;
                    r[6] = uint16_t((r[6] + 2 * nn) & 0xffff);
                    r[7] = r[5];
                    r[5] = read16(getInc(6, 2));
                    return;
                }
                case 7: // sxt: Sign eXTend
                {
                    getDst(&dst, 2);
                    if (trace > 1) debug("sxt", dst);
                    dst.setValue(N ? 0xffff : 0);
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
            reg = (v >> 6) & 7;
            if (trace > 1) debug("mul", dst, reg);
            int src = int16_t(dst.getValue());
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
            reg = (v >> 6) & 7;
            if (trace > 1) debug("div", dst, reg);
            int src = int16_t(dst.getValue());
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
            reg = (v >> 6) & 7;
            if (trace > 1) debug("ash", dst, reg);
            int src = dst.getValue() & 077;
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
            reg = (v >> 6) & 7;
            if (trace > 1) debug("ashc", dst, reg);
            int src = dst.getValue() & 077;
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
            reg = (v >> 6) & 7;
            getDst(&dst, 2);
            if (trace > 1) debug("xor", reg, dst);
            val = r[reg] ^ dst.getValue(true);
            dst.setValue(val);
            setFlags(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 7: // sob: Subtract One from register, Branch if not zero
            reg = (v >> 6) & 7;
            if (trace > 1) debug("sob", reg);
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
            if (trace > 1) debug("bpl");
            r[7] = !N ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x81: // bmi: Branch if MInus
            if (trace > 1) debug("bmi");
            r[7] = N ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x82: // bhi: Branch if HIgher
            if (trace > 1) debug("bhi");
            r[7] = !(C | Z) ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x83: // blos: Branch if LOwer or Same
            if (trace > 1) debug("blos");
            r[7] = C | Z ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x84: // bvc: Branch if oVerflow Clear
            if (trace > 1) debug("bvc");
            r[7] = !V ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x85: // bvs: Branch if oVerflow Set
            if (trace > 1) debug("bvs");
            r[7] = V ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x86: // bcc: Branch if Carry Clear
            if (trace > 1) debug("bcc");
            r[7] = !C ? getOffset(r[7]) : r[7] + 2;
            return;
        case 0x87: // bcs: Branch if Carry Set
            if (trace > 1) debug("bcs");
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
            if (trace > 1) debug("clrb", dst);
            dst.setByte(0);
            setFlags(true, false, false, false);
            return;
        case 051: // comb: COMplement Byte
            getDst(&dst, 1);
            if (trace > 1) debug("comb", dst);
            val = ~dst.getByte(true);
            dst.setByte(val);
            setFlags(val == 0, (val & 0x80) != 0, true, false);
            return;
        case 052: // incb: INCrement Byte
            getDst(&dst, 1);
            if (trace > 1) debug("incb", dst);
            val = int(int8_t(dst.getByte(true))) + 1;
            dst.setByte(val);
            setFlags(val == 0, val < 0, C, val == 0x80);
            return;
        case 053: // decb: DECrement Byte
            getDst(&dst, 1);
            if (trace > 1) debug("decb", dst);
            val = int(int8_t(dst.getByte(true))) - 1;
            dst.setByte(val);
            setFlags(val == 0, val < 0, C, val == -0x81);
            return;
        case 054: // negb: NEGate Byte
        {
            getDst(&dst, 1);
            if (trace > 1) debug("negb", dst);
            int val0 = dst.getByte(true);
            int val1 = -int8_t(val0);
            dst.setByte(val1);
            setFlags(val1 == 0, val1 < 0, val1 != 0, val1 == 0x80);
            return;
        }
        case 055: // adcb: ADd Carry Byte
            getDst(&dst, 1);
            if (trace > 1) debug("adcb", dst);
            val = int(int8_t(dst.getByte(true))) + (C ? 1 : 0);
            dst.setByte(val);
            setFlags(val == 0, val < 0, C && val == 0, val == 0x80);
            return;
        case 056: // sbcb: SuBtract Carry Byte
            getDst(&dst, 1);
            if (trace > 1) debug("sbcb", dst);
            val = int(int8_t(dst.getByte(true))) - (C ? 1 : 0);
            dst.setByte(val);
            setFlags(val == 0, val < 0, C && val == -1, val == -0x81);
            return;
        case 057: // tstb: TeST Byte
            getDst(&dst, 1);
            if (trace > 1) debug("tstb", dst);
            val = int8_t(dst.getByte());
            setFlags(val == 0, val < 0, false, false);
            return;
        case 060: // rorb: ROtate Right Byte
        {
            getDst(&dst, 1);
            if (trace > 1) debug("rorb", dst);
            int val0 = dst.getByte(true);
            int val1 = val0 >> 1;
            if (C) val1 = uint8_t(val1 + 0x80);
            dst.setByte(val1);
            bool lsb0 = (val0 & 1) != 0;
            bool msb1 = C;
            setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
            return;
        }
        case 061: // rolb: ROtate Left Byte
        {
            getDst(&dst, 1);
            if (trace > 1) debug("rolb", dst);
            int val0 = dst.getByte(true);
            int val1 = uint8_t(((uint32_t(val0) << 1) + (C ? 1 : 0)) & 0xff);
            dst.setByte(val1);
            bool msb0 = (val0 & 0x80) != 0;
            bool msb1 = (val1 & 0x80) != 0;
            setFlags(val1 == 0, msb1, msb0, msb1 != msb0);
            return;
        }
        case 062: // asrb: Arithmetic Shift Right Byte
        {
            getDst(&dst, 1);
            if (trace > 1) debug("asrb", dst);
            int val0 = dst.getByte(true);
            int val1 = int8_t(val0) >> 1;
            dst.setByte(val1);
            bool lsb0 = (val0 & 1) != 0;
            bool msb1 = val1 < 0;
            setFlags(val1 == 0, msb1, lsb0, msb1 != lsb0);
            return;
        }
        case 063: // aslb: Arithmetic Shift Left Byte
        {
            getDst(&dst, 1);
            if (trace > 1) debug("aslb", dst);
            int val0 = dst.getByte(true);
            int val1 = uint8_t((uint32_t(val0) << 1) & 0xff);
            dst.setByte(val1);
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
            if (trace > 1) debug("setf");
            r[7] += 2;
            isDouble = false;
            return;
        case 2: // seti: SET Integer
            if (trace > 1) debug("seti");
            r[7] += 2;
            isLong = false;
            return;
        case 011: // setd: SET Double
            if (trace > 1) debug("setd");
            r[7] += 2;
            isDouble = true;
            return;
        case 012: // setl: SET Long
            if (trace > 1) debug("setl");
            r[7] += 2;
            isLong = true;
            return;
    }
    abort("not implemented");
}
