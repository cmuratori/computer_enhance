const std = @import("std");
const c = @cImport({
    @cInclude("sim86_shared.h");
});

extern fn Sim86_GetVersion() u32;
extern fn Sim86_Decode8086Instruction(SourceSize: u32, Source: [*]u8, Dest: *Instruction) void;
extern fn Sim86_RegisterNameFromOperand(RegAccess: *RegisterAccess) [*:0]const u8;
extern fn Sim86_MnemonicFromOperationType(Type: OperationType) [*:0]const u8;
extern fn Sim86_Get8086InstructionTable(Dest: *InstructionTable) void;

pub const OperationType = enum(c_int) {
    Op_None,

    Op_mov,

    Op_push,

    Op_pop,

    Op_xchg,

    Op_in,

    Op_out,

    Op_xlat,
    Op_lea,
    Op_lds,
    Op_les,
    Op_lahf,
    Op_sahf,
    Op_pushf,
    Op_popf,

    Op_add,

    Op_adc,

    Op_inc,

    Op_aaa,
    Op_daa,

    Op_sub,

    Op_sbb,

    Op_dec,

    Op_neg,

    Op_cmp,

    Op_aas,
    Op_das,
    Op_mul,
    Op_imul,
    Op_aam,
    Op_div,
    Op_idiv,
    Op_aad,
    Op_cbw,
    Op_cwd,

    Op_not,
    Op_shl,
    Op_shr,
    Op_sar,
    Op_rol,
    Op_ror,
    Op_rcl,
    Op_rcr,

    Op_and,

    Op_test,

    Op_or,

    Op_xor,

    Op_rep,
    Op_movs,
    Op_cmps,
    Op_scas,
    Op_lods,
    Op_stos,

    Op_call,

    Op_jmp,

    Op_ret,

    Op_retf,

    Op_je,
    Op_jl,
    Op_jle,
    Op_jb,
    Op_jbe,
    Op_jp,
    Op_jo,
    Op_js,
    Op_jne,
    Op_jnl,
    Op_jg,
    Op_jnb,
    Op_ja,
    Op_jnp,
    Op_jno,
    Op_jns,
    Op_loop,
    Op_loopz,
    Op_loopnz,
    Op_jcxz,

    Op_int,
    Op_int3,

    Op_into,
    Op_iret,

    Op_clc,
    Op_cmc,
    Op_stc,
    Op_cld,
    Op_std,
    Op_cli,
    Op_sti,
    Op_hlt,
    Op_wait,
    Op_esc,
    Op_lock,
    Op_segment,

    Op_Count,
};

pub const RegisterAccess = extern struct {
    Index: u32,
    Offset: u32,
    Count: u32,
};

pub const EffectiveAddressTerm = extern struct {
    Register: RegisterAccess,
    Scale: i32,
};

pub const EffectiveAddressFlag = packed struct(u32) {
    ExplicitSegment: bool = false,
    _padding: enum(u31) { unset = 0 } = .unset,
};
pub const EffectiveAddressExpression = extern struct {
    Terms: [2]EffectiveAddressTerm,
    ExplicitSegment: u32,
    Displacement: i32,
    Flags: EffectiveAddressFlag,
};

pub const ImmediateFlag = packed struct(u32) {
    RelativeJumpDisplacement: bool = false,
    _padding: enum(u31) { unset = 0 } = .unset,
};
pub const Immediate = extern struct {
    Value: i32,
    Flags: ImmediateFlag,
};

pub const OperandType = enum(u32) {
    OperandNone,
    OperandRegister,
    OperandMemory,
    OperandImmediate,
};

pub const InstructionOperand = extern struct {
    Type: OperandType,
    data: Data, // <- no anon unions in zig
    const Data = extern union {
        Address: EffectiveAddressExpression,
        Register: RegisterAccess,
        Immediate: Immediate,
    };
};

pub const InstructionFlag = packed struct(u32) {
    Lock: bool = false,
    Rep: bool = false,
    Segment: bool = false,
    Wide: bool = false,
    Far: bool = false,
    // this line could simply be "_padding: u27 = 0", but
    // this way "will be init to 0" is safety checked
    _padding: enum(u27) { unset = 0 } = .unset,
};
pub const Instruction = extern struct {
    Address: u32,
    Size: u32,
    Op: OperationType,
    Flags: InstructionFlag,
    Operands: [2]InstructionOperand,
    SegmentOverride: u32,
};

pub const InstructionBitsUsage = enum(u8) {
    End,

    Literal,

    D,
    S,
    W,
    V,
    Z,
    MOD,
    REG,
    RM,
    SR,
    Disp,
    Data,

    DispAlwaysW,
    WMakesDataW,
    RMRegAlwaysW,
    RelJMPDisp,
    Far,

    Count,
};

pub const InstructionBits = extern struct {
    Usage: InstructionBitsUsage,
    BitCount: u8,
    Shift: u8,
    Value: u8,
};

pub const InstructionEncoding = extern struct {
    Op: OperationType,
    Bits: [16]InstructionBits,
};

pub const InstructionTable = extern struct {
    Encodings: [*]InstructionEncoding,
    EncodingCount: u32,
    MaxInstructionByteCount: u32,
};

pub const DecodeError = error{
    UnrecognizedInstruction,
};

pub fn getVersion() u32 {
    return Sim86_GetVersion();
}

pub fn get8086InstructionTable() InstructionTable {
    var table: InstructionTable = undefined;
    Sim86_Get8086InstructionTable(&table);
    return table;
}

pub fn decode8086Instruction(source: []u8) !Instruction {
    var decoded: Instruction = undefined;
    Sim86_Decode8086Instruction(@intCast(c_uint, source.len), source.ptr, &decoded);
    if (decoded.Op == OperationType.Op_None) {
        return DecodeError.UnrecognizedInstruction;
    }
    return decoded;
}

pub fn mnemonicFromOperationType(operation_type: OperationType) []const u8 {
    const result = Sim86_MnemonicFromOperationType(operation_type);
    const size = std.mem.indexOfSentinel(u8, 0, result);
    return result[0..size];
}

pub fn registerNameFromOperand(register_access: *RegisterAccess) []const u8 {
    const result = Sim86_RegisterNameFromOperand(register_access);
    const size = std.mem.indexOfSentinel(u8, 0, result);
    return result[0..size];
}
