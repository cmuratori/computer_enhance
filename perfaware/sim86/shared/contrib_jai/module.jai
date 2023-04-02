//
// This file was auto-generated using the following command:
//
// jai generate.jai
//
#module_parameters(DEBUG := true);



b32 :: s32;

SIM86_VERSION: u32 : 3;
operation_type :: enum u32 {
    None    :: 0;

    mov     :: 1;

    push    :: 2;

    pop     :: 3;

    xchg    :: 4;

    in      :: 5;

    out     :: 6;

    xlat    :: 7;
    lea     :: 8;
    lds     :: 9;
    les     :: 10;
    lahf    :: 11;
    sahf    :: 12;
    pushf   :: 13;
    popf    :: 14;

    add     :: 15;

    adc     :: 16;

    inc     :: 17;

    aaa     :: 18;
    daa     :: 19;

    sub     :: 20;

    sbb     :: 21;

    dec     :: 22;

    neg     :: 23;

    cmp     :: 24;

    aas     :: 25;
    das     :: 26;
    mul     :: 27;
    imul    :: 28;
    aam     :: 29;
    div     :: 30;
    idiv    :: 31;
    aad     :: 32;
    cbw     :: 33;
    cwd     :: 34;

    not     :: 35;
    shl     :: 36;
    shr     :: 37;
    sar     :: 38;
    rol     :: 39;
    ror     :: 40;
    rcl     :: 41;
    rcr     :: 42;

    and     :: 43;

    test    :: 44;

    or      :: 45;

    xor     :: 46;

    rep     :: 47;
    movs    :: 48;
    cmps    :: 49;
    scas    :: 50;
    lods    :: 51;
    stos    :: 52;

    call    :: 53;

    jmp     :: 54;

    ret     :: 55;

    retf    :: 56;

    je      :: 57;
    jl      :: 58;
    jle     :: 59;
    jb      :: 60;
    jbe     :: 61;
    jp      :: 62;
    jo      :: 63;
    js      :: 64;
    jne     :: 65;
    jnl     :: 66;
    jg      :: 67;
    jnb     :: 68;
    ja      :: 69;
    jnp     :: 70;
    jno     :: 71;
    jns     :: 72;
    loop    :: 73;
    loopz   :: 74;
    loopnz  :: 75;
    jcxz    :: 76;

    int     :: 77;
    int3    :: 78;

    into    :: 79;
    iret    :: 80;

    clc     :: 81;
    cmc     :: 82;
    stc     :: 83;
    cld     :: 84;
    std     :: 85;
    cli     :: 86;
    sti     :: 87;
    hlt     :: 88;
    wait    :: 89;
    esc     :: 90;
    lock    :: 91;
    segment :: 92;

    Count   :: 93;
}

instruction_flag :: enum_flags u32 {
    Lock    :: 0x1;
    Rep     :: 0x2;
    Segment :: 0x4;
    Wide    :: 0x8;
    Far     :: 0x10;
}

register_access :: struct {
    Index:  u32;
    Offset: u32;
    Count:  u32;
}

effective_address_term :: struct {
    Register: register_access;
    Scale:    s32;
}

effective_address_flag :: enum_flags u32 {
    Address_ExplicitSegment :: 0x1;
}

effective_address_expression :: struct {
    Terms:           [2] effective_address_term;
    ExplicitSegment: u32;
    Displacement:    s32;
    Flags:           u32;
}

immediate_flag :: enum_flags u32 {
    Immediate_RelativeJumpDisplacement :: 0x1;
}

immediate :: struct {
    Value: s32;
    Flags: u32;
}

operand_type :: enum u32 {
    None      :: 0;
    Register  :: 1;
    Memory    :: 2;
    Immediate :: 3;
}

instruction_operand :: struct {
    Type: operand_type;
    union {
        Address:   effective_address_expression;
        Register:  register_access;
        Immediate: immediate;
    }
}

instruction :: struct {
    Address:         u32;
    Size:            u32;

    Op:              operation_type;
    Flags:           instruction_flag;

    Operands:        [2] instruction_operand;

    SegmentOverride: u32;
}

instruction_bits_usage :: enum u8 {
    End          :: 0;

    Literal      :: 1;

    D            :: 2;
    S            :: 3;
    W            :: 4;
    V            :: 5;
    Z            :: 6;
    MOD          :: 7;
    REG          :: 8;
    RM           :: 9;
    SR           :: 10;
    Disp         :: 11;
    Data         :: 12;

    DispAlwaysW  :: 13;
    WMakesDataW  :: 14;
    RMRegAlwaysW :: 15;
    RelJMPDisp   :: 16;
    Far          :: 17;

    Count        :: 18;
}

instruction_bits :: struct {
    Usage:    instruction_bits_usage;
    BitCount: u8;
    Shift:    u8;
    Value:    u8;
}

instruction_encoding :: struct {
    Op:   operation_type;
    Bits: [16] instruction_bits;
}

instruction_table :: struct {
    Encodings:               *instruction_encoding;
    EncodingCount:           u32;
    MaxInstructionByteCount: u32;
}

Sim86_GetVersion :: () -> u32 #foreign sim86;
Sim86_Decode8086Instruction :: (SourceSize: u32, Source: *u8, Dest: *instruction) -> void #foreign sim86;
Sim86_RegisterNameFromOperand :: (RegAccess: *register_access) -> *u8 #foreign sim86;
Sim86_MnemonicFromOperationType :: (Type: operation_type) -> *u8 #foreign sim86;
Sim86_Get8086InstructionTable :: (Dest: *instruction_table) -> void #foreign sim86;

#scope_file

#if DEBUG == true {
    sim86 :: #library "../sim86_shared_debug";
} else #if DEBUG == false {
    sim86 :: #library "../sim86_shared_release";
}
