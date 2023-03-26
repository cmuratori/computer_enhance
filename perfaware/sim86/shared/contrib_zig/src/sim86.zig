const std = @import("std");
const c = @cImport({
    @cInclude("sim86_shared.h");
});

pub const InstructionTable = c.instruction_table;
pub const Instruction = c.instruction;
pub const OperationType = c.operation_type;
pub const RegisterAccess = c.register_access;
pub const OperandType = enum(u32) {
    OperandNone,
    OperandRegister,
    OperandMemory,
    OperandImmediate,
};

pub const DecodeError = error{
    UnrecognizedInstruction,
};

pub fn getVersion() u32 {
    return c.Sim86_GetVersion();
}

pub fn get8086InstructionTable() InstructionTable {
    var table: c.instruction_table = undefined;
    c.Sim86_Get8086InstructionTable(&table);
    return table;
}

pub fn decode8086Instruction(source: []u8) !Instruction {
    var decoded: c.instruction = undefined;
    c.Sim86_Decode8086Instruction(@intCast(c_uint, source.len), source.ptr, &decoded);
    if (decoded.Op == 0) {
        return DecodeError.UnrecognizedInstruction;
    }
    return decoded;
}

pub fn mnemonicFromOperationType(operation_type: OperationType) []const u8 {
    const result = c.Sim86_MnemonicFromOperationType(operation_type);
    const size = std.mem.indexOfSentinel(u8, 0, result);
    return result[0..size];
}

pub fn registerNameFromOperand(register_access: *RegisterAccess) []const u8 {
    const result = c.Sim86_RegisterNameFromOperand(register_access);
    const size = std.mem.indexOfSentinel(u8, 0, result);
    return result[0..size];
}
