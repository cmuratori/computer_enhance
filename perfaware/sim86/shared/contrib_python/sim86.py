# place "sim86_shared_debug.dll" next to this file

import ctypes
import pathlib
import typing
from enum import IntEnum, IntFlag, EnumType
from dataclasses import dataclass, fields

### public interface

VERSION = 3

OperationType = IntEnum("OperationType", """
  none mov push pop xchg in out xlat lea lds les lahf sahf
  pushf popf add adc inc aaa daa sub sbb dec neg cmp aas
  das mul imul aam div idiv aad cbw cwd not shl shr sar rol
  ror rcl rcr and test or xor rep movs cmps scas lods stos
  call jmp ret retf je jl jle jb jbe jp jo js jne jnl jg jnb
  ja jnp jno jns loop loopz loopnz jcxz int int3 into iret
  clc cmc stc cld std cli sti hlt wait esc lock segment
""".split(), start=0)
 
InstructionFlag = IntFlag("InstructionFlag", """
  lock rep segment wide far
""".split())

EffectiveAddressFlag = IntFlag("EffectiveAddressFlag", """
  explicit_segment
""".split())

ImmediateFlag = IntFlag("ImmediateFlag", """
  relative_jump_displacement
""".split())

InstructionBitsUsage = IntEnum("InstructionBitsUsage", """
  end literal d s w v z mod reg rm sr disp data
  disp_always_w w_makes_data_w rm_reg_always_w
  rel_jump_disp far
""".split(), start=0)

@dataclass
class RegisterAccess:
  index: int
  offset: int
  count: int

@dataclass
class EffectiveAddressTerm:
  register: RegisterAccess
  scale: int

@dataclass
class EffectiveAddressExpression:
  terms: list[EffectiveAddressTerm]
  explicit_segment: int
  displacement: int
  flags: EffectiveAddressFlag

@dataclass
class Immediate:
  value: int
  flags: ImmediateFlag

@dataclass
class Instruction:
  address: int
  size: int
  op: OperationType
  flags: InstructionFlag
  operands: list[typing.Union[EffectiveAddressExpression, RegisterAccess, Immediate]]
  segment_override: int

@dataclass
class InstructionBits:
  usage: InstructionBitsUsage
  bit_count: int
  shift: int
  value: int

  @staticmethod
  def _convert(arg):
    return 

@dataclass
class InstructionEncoding:
  op: OperationType
  bits: list[InstructionBits]

@dataclass
class InstructionTable:
  encodings: list[InstructionEncoding]
  max_instruction_byte_count: int


def get_version() -> int:
  return _get_version()


def decode_8086_instruction(data: bytes, offset: int) -> Instruction:
  assert isinstance(data, bytes)
  decoded = _instruction()
  length = len(data) - offset
  ptr = ctypes.cast(data, ctypes.POINTER(ctypes.c_ubyte))
  ptr = ctypes.addressof(ptr.contents) + offset
  _decode_8086_instruction(length, ptr, ctypes.byref(decoded))
  return _make(decoded)

def register_name_from_operand(register_access: RegisterAccess) -> str:
  access = _register_access(register_access.index, register_access.offset, register_access.count)
  return _register_name_from_operand(ctypes.byref(access)).decode("ascii")

def mnemonic_from_operation_type(op: OperationType) -> str:
  return _mnemonic_from_operation_type(op).decode("ascii")

def get_8086_instruction_table() -> InstructionTable:
  t = _instruction_table()
  _get_8086_instruction_table(ctypes.byref(t))
  return _make(t)


### implementation details


u8 = ctypes.c_ubyte
u32 = ctypes.c_uint
s32 = ctypes.c_int

_operand_type = IntEnum("OperandType", """
  none register memory immediate
""".split(), start=0)

class _register_access(ctypes.Structure):
  _fields_ = [("index", u32), 
              ("offset", u32),
              ("count", u32)]
  _type_ = RegisterAccess

class _effective_address_term(ctypes.Structure):
  _fields_ = [("register", _register_access), 
              ("scale", s32)]
  _type_ = EffectiveAddressTerm

class _effective_address_expression(ctypes.Structure):
  _fields_ = [("terms", _effective_address_term * 2), 
              ("explicit_segment", u32),
              ("displacement", s32),
              ("flags", u32)] # EffectiveAddressFlag
  def _convert(self):
    return EffectiveAddressExpression([_make(term) for term in self.terms], self.explicit_segment, self.displacement, EffectiveAddressFlag(self.flags))

class _immediate(ctypes.Structure):
  _fields_ = [("value", s32), 
              ("flags", u32)] # ImmediateFlag
  _type_ = Immediate

class _instruction_operand_union(ctypes.Union):
  _fields_ = [("address", _effective_address_expression),
              ("register", _register_access),
              ("immediate", _immediate)]

class _instruction_operand(ctypes.Structure):
  _anonymous_ = ["u"]
  _fields_ = [("type", u32), # OperandType
              ("u", _instruction_operand_union)]
  def _convert(self):
    if self.type == _operand_type.register:    return _make(self.register)
    elif self.type == _operand_type.memory:    return _make(self.address)
    elif self.type == _operand_type.immediate: return _make(self.immediate)
    else: assert False

class _instruction(ctypes.Structure):
  _fields_ = [("address", u32), 
              ("size", u32),
              ("op", u32), # OperationType
              ("flags", u32), # InstructionFlag
              ("operands", _instruction_operand * 2),
              ("segment_override", u32)]
  def _convert(self):
    operands = [op._convert() for op in self.operands if op.type != _operand_type.none]
    return Instruction(self.address, self.size, OperationType(self.op), InstructionFlag(self.flags), operands, self.segment_override)

class _instruction_bits(ctypes.Structure):
  _fields_ = [("usage", u8), # InstructionBitsUsage
              ("bit_count", u8),
              ("shift", u8),
              ("value", u8)]
  _type_ = InstructionBits

class _instruction_encoding(ctypes.Structure):
  _fields_ = [("op", u32), # OperationType
              ("bits", _instruction_bits * 16)]
  def _convert(self):
    return InstructionEncoding(OperationType(self.op), [_make(x) for x in self.bits])
  
class _instruction_table(ctypes.Structure):
  _fields_ = [("encodings", ctypes.POINTER(_instruction_encoding)), 
              ("encoding_count", u32),
              ("max_instruction_byte_count", u32)]
  def _convert(self):
    return InstructionTable([_make(self.encodings[i]) for i in range(self.encoding_count)], self.max_instruction_byte_count)

dll = ctypes.CDLL(str(pathlib.Path(__file__).parent / "sim86_shared_debug.dll"))

_get_version = dll.Sim86_GetVersion
_get_version.argtypes = []
_get_version.restype = u32

_decode_8086_instruction = dll.Sim86_Decode8086Instruction
_decode_8086_instruction.argtypes = [u32, ctypes.c_void_p, ctypes.POINTER(_instruction)]

_register_name_from_operand = dll.Sim86_RegisterNameFromOperand
_register_name_from_operand.argtypes = [ctypes.POINTER(_register_access)]
_register_name_from_operand.restype = ctypes.c_char_p

_mnemonic_from_operation_type = dll.Sim86_MnemonicFromOperationType
_mnemonic_from_operation_type.argtypes = [u32] # OperationType
_mnemonic_from_operation_type.restype = ctypes.c_char_p

_get_8086_instruction_table = dll.Sim86_Get8086InstructionTable
_get_8086_instruction_table.argtypes = [ctypes.POINTER(_instruction_table)]

### helper function to convert ctypes -> dataclass

def _make(obj):
  if isinstance(obj, int):     return obj
  if hasattr(obj, "_convert"): return obj._convert()

  dst = dict((f.name, f) for f in fields(obj._type_))

  args = {}
  for field in obj._fields_:
    value = getattr(obj, field[0])
    ftype = dst[field[0]].type
    if isinstance(ftype, EnumType):
      args[field[0]] = ftype(value)
    else:
      args[field[0]] = _make(value)

  return obj._type_(**args)
