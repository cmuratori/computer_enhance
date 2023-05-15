/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/*
   NOTE(casey): This instruction table is a direct translation of table 4-12 in the Intel 8086 manual.
   The macros are designed to allow direct transcription, without changing the order or manner
   of specification in the table in any way. Additional "implicit" versions of the macros are provided
   so that hard-coded fields can be supplied uniformly.
   
   The table is also designed to allow you to include it multiple times to "pull out" other things
   from the table, such as opcode mnemonics as strings or enums, etc.
*/

#ifndef INST
#define INST(Mnemonic, ...) {Op_##Mnemonic, __VA_ARGS__},
#endif

#ifndef INSTALT
#define INSTALT INST
#endif

#define B(Bits) {Bits_Literal, sizeof(#Bits)-1, 0, 0b##Bits}
#define D {Bits_D, 1}
#define S {Bits_S, 1}
#define W {Bits_W, 1}
#define V {Bits_V, 1}
#define Z {Bits_Z, 1}

#define XXX {Bits_Data, 3, 0}
#define YYY {Bits_Data, 3, 3}
#define RM {Bits_RM, 3}
#define MOD {Bits_MOD, 2}
#define REG {Bits_REG, 3}
#define SR {Bits_SR, 2}

#define ImpW(Value) {Bits_W, 0, 0, Value}
#define ImpREG(Value) {Bits_REG, 0, 0, Value}
#define ImpMOD(Value) {Bits_MOD, 0, 0, Value}
#define ImpRM(Value) {Bits_RM, 0, 0, Value}
#define ImpD(Value) {Bits_D, 0, 0, Value}
#define ImpS(Value) {Bits_S, 0, 0, Value}
    
#define DISP {Bits_Disp, 0, 0, 0}
#define ADDR {Bits_Disp, 0, 0, 0}, {Bits_DispAlwaysW, 0, 0, 1}
#define DATA {Bits_Data, 0, 0, 0}
#define DATA_IF_W {Bits_WMakesDataW, 0, 0, 1}
#define Flags(F) {F, 0, 0, 1}

INST(mov, {B(100010), D, W, MOD, REG, RM})
INSTALT(mov, {B(1100011), W, MOD, B(000), RM, DATA, DATA_IF_W, ImpD(0)})
INSTALT(mov, {B(1011), W, REG, DATA, DATA_IF_W, ImpD(1)})
INSTALT(mov, {B(1010000), W, ADDR, ImpREG(0), ImpMOD(0), ImpRM(0b110), ImpD(1)})
INSTALT(mov, {B(1010001), W, ADDR, ImpREG(0), ImpMOD(0), ImpRM(0b110), ImpD(0)})
INSTALT(mov, {B(100011), D, B(0), MOD, B(0), SR, RM, ImpW(1)}) // NOTE(casey): This collapses 2 entries in the 8086 table by adding an explicit D bit

INST(push, {B(11111111), MOD, B(110), RM, ImpW(1), ImpD(1)})
INSTALT(push, {B(01010), REG, ImpW(1), ImpD(1)})
INSTALT(push, {B(000), SR, B(110), ImpW(1), ImpD(1)})

INST(pop, {B(10001111), MOD, B(000), RM, ImpW(1), ImpD(1)})
INSTALT(pop, {B(01011), REG, ImpW(1), ImpD(1)})
INSTALT(pop, {B(000), SR, B(111), ImpW(1), ImpD(1)})

INST(xchg, {B(1000011), W, MOD, REG, RM, ImpD(1)})
INSTALT(xchg, {B(10010), REG, ImpMOD(0b11), ImpW(1), ImpRM(0)})

INST(in, {B(1110010), W, DATA, ImpREG(0), ImpD(1)})
INSTALT(in, {B(1110110), W, ImpREG(0), ImpD(1), ImpMOD(0b11), ImpRM(2), Flags(Bits_RMRegAlwaysW)})
INST(out, {B(1110011), W, DATA, ImpREG(0), ImpD(0)})
INSTALT(out, {B(1110111), W, ImpREG(0), ImpD(0), ImpMOD(0b11), ImpRM(2), Flags(Bits_RMRegAlwaysW)})

INST(xlat, {B(11010111)})
INST(lea, {B(10001101), MOD, REG, RM, ImpD(1), ImpW(1)})
INST(lds, {B(11000101), MOD, REG, RM, ImpD(1), ImpW(1)})
INST(les, {B(11000100), MOD, REG, RM, ImpD(1), ImpW(1)})
INST(lahf, {B(10011111)})
INST(sahf, {B(10011110)})
INST(pushf, {B(10011100)})
INST(popf, {B(10011101)})

INST(add, {B(000000), D, W, MOD, REG, RM})
INSTALT(add, {B(100000), S, W, MOD, B(000), RM, DATA, DATA_IF_W})
INSTALT(add, {B(0000010), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(adc, {B(000100), D, W, MOD, REG, RM})
INSTALT(adc, {B(100000), S, W, MOD, B(010), RM, DATA, DATA_IF_W})
INSTALT(adc, {B(0001010), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(inc, {B(1111111), W, MOD, B(000), RM, ImpD(1)})
INSTALT(inc, {B(01000), REG, ImpW(1), ImpD(1)})

INST(aaa, {B(00110111)})
INST(daa, {B(00100111)})

INST(sub, {B(001010), D, W, MOD, REG, RM})
INSTALT(sub, {B(100000), S, W, MOD, B(101), RM, DATA, DATA_IF_W})
INSTALT(sub, {B(0010110), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(sbb, {B(000110), D, W, MOD, REG, RM})
INSTALT(sbb, {B(100000), S, W, MOD, B(011), RM, DATA, DATA_IF_W})
INSTALT(sbb, {B(0001110), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(dec, {B(1111111), W, MOD, B(001), RM, ImpD(1)})
INSTALT(dec, {B(01001), REG, ImpW(1), ImpD(1)})

INST(neg, {B(1111011), W, MOD, B(011), RM})

INST(cmp, {B(001110), D, W, MOD, REG, RM})
INSTALT(cmp, {B(100000), S, W, MOD, B(111), RM, DATA, DATA_IF_W})
INSTALT(cmp, {B(0011110), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)}) // NOTE(casey): The manual table suggests this data is only 8-bit, but wouldn't it be 16 as well?

INST(aas, {B(00111111)})
INST(das, {B(00101111)})
INST(mul, {B(1111011), W, MOD, B(100), RM, ImpS(0)})
INST(imul, {B(1111011), W, MOD, B(101), RM, ImpS(1)})
INST(aam, {B(11010100), B(00001010)}) // NOTE(casey): The manual says this has a DISP... but how could it? What for??
INST(div, {B(1111011), W, MOD, B(110), RM, ImpS(0)})
INST(idiv, {B(1111011), W, MOD, B(111), RM, ImpS(1)})
INST(aad, {B(11010101), B(00001010)})
INST(cbw, {B(10011000)})
INST(cwd, {B(10011001)})

INST(not, {B(1111011), W, MOD, B(010), RM})
INST(shl, {B(110100), V, W, MOD, B(100), RM})
INST(shr, {B(110100), V, W, MOD, B(101), RM})
INST(sar, {B(110100), V, W, MOD, B(111), RM})
INST(rol, {B(110100), V, W, MOD, B(000), RM})
INST(ror, {B(110100), V, W, MOD, B(001), RM})
INST(rcl, {B(110100), V, W, MOD, B(010), RM})
INST(rcr, {B(110100), V, W, MOD, B(011), RM})

INST(and, {B(001000), D, W, MOD, REG, RM})
INSTALT(and, {B(1000000), W, MOD, B(100), RM, DATA, DATA_IF_W})
INSTALT(and, {B(0010010), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(test, {B(1000010), W, MOD, REG, RM}) // NOTE(casey): The manual suggests there is a D flag here, but it doesn't appear to be true (it would conflict with xchg if it did)
INSTALT(test, {B(1111011), W, MOD, B(000), RM, DATA, DATA_IF_W})
INSTALT(test, {B(1010100), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)}) // NOTE(casey): The manual table suggests this data is only 8-bit, but it seems like it could be 16 too?

INST(or, {B(000010), D, W, MOD, REG, RM})
INSTALT(or, {B(1000000), W, MOD, B(001), RM, DATA, DATA_IF_W})
INSTALT(or, {B(0000110), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(xor, {B(001100), D, W, MOD, REG, RM})
INSTALT(xor, {B(1000000), W, MOD, B(110), RM, DATA, DATA_IF_W}) // NOTE(casey): The manual has conflicting information about this encoding, but I believe this is the correct binary pattern.
INSTALT(xor, {B(0011010), W, DATA, DATA_IF_W, ImpREG(0), ImpD(1)})

INST(rep, {B(1111001), Z})
INST(movs, {B(1010010), W})
INST(cmps, {B(1010011), W})
INST(scas, {B(1010111), W})
INST(lods, {B(1010110), W})
INST(stos, {B(1010101), W})

INST(call, {B(11101000), ADDR, Flags(Bits_RelJMPDisp)})
INSTALT(call, {B(11111111), MOD, B(010), RM, ImpW(1)})
INSTALT(call, {B(10011010), ADDR, DATA, DATA_IF_W, ImpW(1), Flags(Bits_Far)})
INSTALT(call, {B(11111111), MOD, B(011), RM, ImpW(1), Flags(Bits_Far)})

INST(jmp, {B(11101001), ADDR, Flags(Bits_RelJMPDisp)})
INSTALT(jmp, {B(11101011), DISP, Flags(Bits_RelJMPDisp)})
INSTALT(jmp, {B(11111111), MOD, B(100), RM, ImpW(1)})
INSTALT(jmp, {B(11101010), ADDR, DATA, DATA_IF_W, ImpW(1), Flags(Bits_Far)})
INSTALT(jmp, {B(11111111), MOD, B(101), RM, ImpW(1), Flags(Bits_Far)})

// NOTE(casey): The actual Intel manual does not distinguish mnemonics RET and RETF,
// but NASM needs this to reassemble properly, so we do.
INST(ret, {B(11000011)})
INSTALT(ret, {B(11000010), DATA, DATA_IF_W, ImpW(1)})
INST(retf, {B(11001011), Flags(Bits_Far)})
INSTALT(retf, {B(11001010), DATA, DATA_IF_W, ImpW(1), Flags(Bits_Far)})

INST(je, {B(01110100), DISP, Flags(Bits_RelJMPDisp)})
INST(jl, {B(01111100), DISP, Flags(Bits_RelJMPDisp)})
INST(jle, {B(01111110), DISP, Flags(Bits_RelJMPDisp)})
INST(jb, {B(01110010), DISP, Flags(Bits_RelJMPDisp)})
INST(jbe, {B(01110110), DISP, Flags(Bits_RelJMPDisp)})
INST(jp, {B(01111010), DISP, Flags(Bits_RelJMPDisp)})
INST(jo, {B(01110000), DISP, Flags(Bits_RelJMPDisp)})
INST(js, {B(01111000), DISP, Flags(Bits_RelJMPDisp)})
INST(jne, {B(01110101), DISP, Flags(Bits_RelJMPDisp)})
INST(jnl, {B(01111101), DISP, Flags(Bits_RelJMPDisp)})
INST(jg, {B(01111111), DISP, Flags(Bits_RelJMPDisp)})
INST(jnb, {B(01110011), DISP, Flags(Bits_RelJMPDisp)})
INST(ja, {B(01110111), DISP, Flags(Bits_RelJMPDisp)})
INST(jnp, {B(01111011), DISP, Flags(Bits_RelJMPDisp)})
INST(jno, {B(01110001), DISP, Flags(Bits_RelJMPDisp)})
INST(jns, {B(01111001), DISP, Flags(Bits_RelJMPDisp)})
INST(loop, {B(11100010), DISP, Flags(Bits_RelJMPDisp)})
INST(loopz, {B(11100001), DISP, Flags(Bits_RelJMPDisp)})
INST(loopnz, {B(11100000), DISP, Flags(Bits_RelJMPDisp)})
INST(jcxz, {B(11100011), DISP, Flags(Bits_RelJMPDisp)})

INST(int, {B(11001101), DATA})
INST(int3, {B(11001100)}) // TODO(casey): The manual does not suggest that this intrinsic has an "int3" mnemonic, but NASM thinks so

INST(into, {B(11001110)})
INST(iret, {B(11001111)})

INST(clc, {B(11111000)})
INST(cmc, {B(11110101)})
INST(stc, {B(11111001)})
INST(cld, {B(11111100)})
INST(std, {B(11111101)})
INST(cli, {B(11111010)})
INST(sti, {B(11111011)})
INST(hlt, {B(11110100)})
INST(wait, {B(10011011)})
INST(esc, {B(11011), XXX, MOD, YYY, RM})
INST(lock, {B(11110000)})
INST(segment, {B(001), SR, B(110)})

#undef INST
#undef INSTALT

#undef B
#undef D
#undef S
#undef W
#undef V
#undef Z

#undef XXX
#undef YYY
#undef RM
#undef MOD
#undef REG
#undef SR

#undef ImpW
#undef ImpREG
#undef ImpMOD
#undef ImpRM
#undef ImpD
#undef ImpS

#undef DISP
#undef ADDR
#undef DATA
#undef DATA_IF_W
#undef Flags
