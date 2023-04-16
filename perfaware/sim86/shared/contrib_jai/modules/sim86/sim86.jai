// sim86.jai

#module_parameters(DEBUG := false);

b32 :: #type,distinct s32;

SIM86_VERSION : u32 : 3;

operation_type :: enum u32 {
    Op_None;

    Op_mov;

    Op_push;

    Op_pop;

    Op_xchg;

    Op_in;

    Op_out;

    Op_xlat;
    Op_lea;
    Op_lds;
    Op_les;
    Op_lahf;
    Op_sahf;
    Op_pushf;
    Op_popf;

    Op_add;

    Op_adc;

    Op_inc;

    Op_aaa;
    Op_daa;

    Op_sub;

    Op_sbb;

    Op_dec;

    Op_neg;

    Op_cmp;

    Op_aas;
    Op_das;
    Op_mul;
    Op_imul;
    Op_aam;
    Op_div;
    Op_idiv;
    Op_aad;
    Op_cbw;
    Op_cwd;

    Op_not;
    Op_shl;
    Op_shr;
    Op_sar;
    Op_rol;
    Op_ror;
    Op_rcl;
    Op_rcr;

    Op_and;

    Op_test;

    Op_or;

    Op_xor;

    Op_rep;
    Op_movs;
    Op_cmps;
    Op_scas;
    Op_lods;
    Op_stos;

    Op_call;

    Op_jmp;

    Op_ret;

    Op_retf;

    Op_je;
    Op_jl;
    Op_jle;
    Op_jb;
    Op_jbe;
    Op_jp;
    Op_jo;
    Op_js;
    Op_jne;
    Op_jnl;
    Op_jg;
    Op_jnb;
    Op_ja;
    Op_jnp;
    Op_jno;
    Op_jns;
    Op_loop;
    Op_loopz;
    Op_loopnz;
    Op_jcxz;

    Op_int;
    Op_int3;

    Op_into;
    Op_iret;

    Op_clc;
    Op_cmc;
    Op_stc;
    Op_cld;
    Op_std;
    Op_cli;
    Op_sti;
    Op_hlt;
    Op_wait;
    Op_esc;
    Op_lock;
    Op_segment;

    Op_Count;
};

instruction_flag :: enum_flags u32 {
    Inst_Lock :: 0x1;
    Inst_Rep :: 0x2;
    Inst_Segment :: 0x4;
    Inst_Wide :: 0x8;
    Inst_Far :: 0x10;
};

register_access :: struct {
    Index: u32;
    Offset: u32;
    Count: u32;
};

effective_address_term :: struct {
    Register: register_access;
    Scale: s32;
};

effective_address_flag :: enum_flags u32 {
    Address_ExplicitSegment :: 0x1;
};

effective_address_expression :: struct {
    Terms: [2] effective_address_term;
    ExplicitSegment: u32;
    Displacement: s32;
    Flags: u32;
};

immediate_flag :: enum_flags u32 {
    Immediate_RelativeJumpDisplacement :: 0x1;
};

immediate :: struct {
    Value: s32;
    Flags: u32;
};

operand_type :: enum u32 {
    Operand_None;
    Operand_Register;
    Operand_Memory;
    Operand_Immediate;
};

instruction_operand :: struct {
    Type: operand_type;
    union {
        Address: effective_address_expression;
        Register: register_access;
        Immediate: immediate;
    };
};

instruction :: struct {
    Address: u32;
    Size: u32;

    Op: operation_type;
    Flags: u32;

    Operands: [2] instruction_operand;

    SegmentOverride: u32;
};

instruction_bits_usage :: enum u8 {
    Bits_End;

    Bits_Literal;

    Bits_D;
    Bits_S;
    Bits_W;
    Bits_V;
    Bits_Z;
    Bits_MOD;
    Bits_REG;
    Bits_RM;
    Bits_SR;
    Bits_Disp;
    Bits_Data;

    Bits_DispAlwaysW;
    Bits_WMakesDataW;
    Bits_RMRegAlwaysW;
    Bits_RelJMPDisp;
    Bits_Far;

    Bits_Count;
};

instruction_bits :: struct {
    Usage: instruction_bits_usage;
    BitCount: u8;
    Shift: u8;
    Value: u8;
};

instruction_encoding :: struct {
    Op: operation_type;
    Bits: [16] instruction_bits;
};

instruction_table :: struct {
    Encodings: *instruction_encoding;
    EncodingCount: u32;
    MaxInstructionByteCount: u32;
};

Sim86_GetVersion :: () -> u32 #foreign SIM86;
Sim86_Decode8086Instruction :: (SourceSize: u32, Source: *u8, Dest: *instruction) #foreign SIM86;
Sim86_RegisterNameFromOperand :: (RegAccess: *register_access) -> *u8 #foreign SIM86;
Sim86_MnemonicFromOperationType :: (Type: operation_type) -> *u8 #foreign SIM86;
Sim86_Get8086InstructionTable :: (Dest: *instruction_table) #foreign SIM86;

