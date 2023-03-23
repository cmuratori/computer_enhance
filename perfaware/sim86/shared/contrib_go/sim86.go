package main

import (
	"syscall"
	"fmt"
	"unsafe"
)

type OperationType uint32
const (
	OpNone OperationType = iota
	OpMov
	OpPush
	OpPop
	OpXchg
	OpIn
	OpOut
	OpXlat
	OpLea
	OpLds
	OpLes
	OpLahf
	OpSahf
	OpPushf
	OpPopf
	OpAdd
	OpAdc
	OpInc
	OpAaa
	OpDaa
	OpSub
	OpSbb
	OpDec
	OpNeg
	OpCmp
	OpAas
	OpDas
	OpMul
	OpImul
	OpAam
	OpDiv
	OpIdiv
	OpAad
	OpCbw
	OpCwd
	OpNot
	OpShl
	OpShr
	OpSar
	OpRol
	OpRor
	OpRcl
	OpRcr
	OpAnd
	OpTest
	OpOr
	OpXor
	OpRep
	OpMovs
	OpCmps
	OpScas
	OpLods
	OpStos
	OpCall
	OpJmp
	OpRet
	OpRetf
	OpJe
	OpJl
	OpJle
	OpJb
	OpJbe
	OpJp
	OpJo
	OpJs
	OpJne
	OpJnl
	OpJg
	OpJnb
	OpJa
	OpJnp
	OpJno
	OpJns
	OpLoop
	OpLoopz
	OpLoopnz
	OpJcxz
	OpInt
	OpInt3
	OpInto
	OpIret
	OpClc
	OpCmc
	OpStc
	OpCld
	OpStd
	OpCli
	OpSti
	OpHlt
	OpWait
	OpEsc
	OpLock
	OpSegment
	OpCount
)

type InstructionFlag uint32
const (
	InstLock    InstructionFlag = 0x01
	InstRep     InstructionFlag = 0x02
	InstSegment InstructionFlag = 0x04
	InstWide    InstructionFlag = 0x08
	InstFar     InstructionFlag = 0x10
)

type RegisterAccess struct {
	Index  uint32
	Offset uint32
	Count  uint32
}

type EffectiveAddressTerm struct {
	Register RegisterAccess
	Scale    uint32
}

type EffectiveAddressFlag uint32
const (
	AddressExplicitSegment EffectiveAddressFlag = 0x1
)

type EffectiveAddressExpression struct {
	Terms           [2]EffectiveAddressTerm
	ExplicitSegment uint32
	Displacement    int32
	Flags           uint32

}

type ImmediateFlag uint32
const (
	ImmediateRelativeJumpDisplacement ImmediateFlag = 0x1
)

type Immediate struct {
	Value int32
	Flags uint32
}

type OperandType uint32
const (
	OperandNone OperandType = iota
	OperandRegister
	OperandMemory
	OperandImmediate
)

type InstructionOperand struct {
	Type OperandType
	// golang doesn't have unions
	data [44]byte
}

func (io *InstructionOperand) GetAddress() EffectiveAddressExpression {
	return *(*EffectiveAddressExpression)(unsafe.Pointer(&io.data[0]))
}

func (io *InstructionOperand) SetAddress(address EffectiveAddressExpression) {
	*(*EffectiveAddressExpression)(unsafe.Pointer(&io.data[0])) = address;
}

func (io *InstructionOperand) GetRegister() RegisterAccess {
	return *(*RegisterAccess)(unsafe.Pointer(&io.data[0]))
}

func (io *InstructionOperand) SetRegister(register RegisterAccess) {
	*(*RegisterAccess)(unsafe.Pointer(&io.data[0])) = register;
}

func (io *InstructionOperand) GetImmediate() Immediate {
	return *(*Immediate)(unsafe.Pointer(&io.data[0]))
}

func (io *InstructionOperand) SetImmediate(immediate Immediate) {
	*(*Immediate)(unsafe.Pointer(&io.data[0])) = immediate;
}

type Instruction struct {
	Address         uint32
	Size            uint32
	Op              OperationType
	Flags           uint32
	Operands        [2]InstructionOperand
	SegmentOverride uint32
}

type InstructionBitsUsage uint8
const (
	BitsEnd InstructionBitsUsage = iota
	BitsLiteral
	BitsD
	BitsS
	BitsW
	BitsV
	BitsZ
	BitsMOD
	BitsREG
	BitsRM
	BitsSR
	BitsDisp
	BitsData
	BitsDispAlwaysW
	BitsWMakesDataW
	BitsRMRegAlwaysW
	BitsRelJMPDisp
	BitsFar
	BitsCount
)

type InstructionBits struct {
	Usage    InstructionBitsUsage
	BitCount uint8
	Shift    uint8
	Value    uint8
}

type InstructionEncoding struct {
	Op   OperationType
	Bits [16]InstructionBits
}

type InstructionTable struct {
	Encodings               *InstructionEncoding
	EncodingCount           uint32
	MaxInstructionByteCount uint32
}

type Sim86 struct {
	dll *syscall.DLL
}

// util function that converts raw null-terminated string into Golang's string
func uintptrToString(ptr uintptr) string {
	buf := make([]byte, 0, 32)
	for {
		c := byte(*(*uint8)(unsafe.Pointer(ptr)))
		if c == 0 {
			break
		}
		buf = append(buf, c)
		ptr++
	}

	return string(buf)
}

func LoadSim86(dllPath string) (s *Sim86, err error) {
	s = new(Sim86)
	s.dll, err = syscall.LoadDLL(dllPath)

	return
}

func (s *Sim86) GetVersion() (uint32, error) {
	getVersion, err := s.dll.FindProc("Sim86_GetVersion")
	if err != nil {
		return 0, err
	}

	ret, _, err := getVersion.Call()
	if err != nil && err != syscall.Errno(0) {
		fmt.Println("Error calling Sim86_GetVersion:", err)
		return 0, err
	}

	return uint32(ret), nil
}

func (s *Sim86) Decode8086Instruction(SourceSize uint32, Source *uint8, Dest *Instruction) error {
	decodeInstruction, err := s.dll.FindProc("Sim86_Decode8086Instruction")
	if err != nil {
		return err
	}

	_, _, err = decodeInstruction.Call(
		uintptr(SourceSize),
		uintptr(unsafe.Pointer(Source)),
		uintptr(unsafe.Pointer(Dest)),
	)

	if err != nil && err != syscall.Errno(0) {
		fmt.Println("Error calling Sim86_Decode8086Instruction:", err)
		return err
	}

	return nil
}

func (s *Sim86) RegisterNameFromOperand(RegAccess *RegisterAccess) (string, error) {
	registerName, err := s.dll.FindProc("Sim86_RegisterNameFromOperand")
	if err != nil {
		return "", err
	}

	ret, _, err := registerName.Call(uintptr(unsafe.Pointer(RegAccess)))
	if err != nil && err != syscall.Errno(0) {
		fmt.Println("Error calling Sim86_RegisterNameFromOperand:", err)
		return "", err
	}

	return uintptrToString(ret), nil
}

func (s *Sim86) MnemonicFromOperationType(Type OperationType) (string, error) {
	mnemonic, err := s.dll.FindProc("Sim86_MnemonicFromOperationType")
	if err != nil {
		return "", err
	}

	ret, _, err := mnemonic.Call(uintptr(Type))
	if err != nil && err != syscall.Errno(0) {
		fmt.Println("Error calling Sim86_MnemonicFromOperationType:", err)
		return "", err
	}

	return uintptrToString(ret), nil
}

func (s *Sim86) Get8086InstructionTable(Dest *InstructionTable) error {
	getTable, err := s.dll.FindProc("Sim86_Get8086InstructionTable")
	if err != nil {
		return err
	}

	_, _, err = getTable.Call(uintptr(unsafe.Pointer(Dest)))
	if err != nil && err != syscall.Errno(0) {
		fmt.Println("Error calling Sim86_Get8086InstructionTable")
		return err
	}

	return nil
}

