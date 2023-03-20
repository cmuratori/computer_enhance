package sim86


when ODIN_OS == .Windows { foreign import sim86 "./sim86_shared_debug.lib" }
when ODIN_OS == .Linux { foreign import sim86 "./sim86_shared_debug.a" }

SIM86_VERSION : u32 : 3

Operation_Type :: enum u32 {
	None,

	mov,

	push,

	pop,

	xchg,

	_in,

	out,

	xlat,
	lea,
	lds,
	les,
	lahf,
	sahf,
	pushf,
	popf,

	add,

	adc,

	inc,

	aaa,
	daa,

	sub,

	sbb,

	dec,

	neg,

	cmp,

	aas,
	das,
	mul,
	imul,
	aam,
	div,
	idiv,
	aad,
	cbw,
	cwd,

	not,
	shl,
	shr,
	sar,
	rol,
	ror,
	rcl,
	rcr,

	and,

	test,

	or,

	xor,

	rep,
	movs,
	cmps,
	scas,
	lods,
	stos,

	call,

	jmp,

	ret,

	retf,

	je,
	jl,
	jle,
	jb,
	jbe,
	jp,
	jo,
	js,
	jne,
	jnl,
	jg,
	jnb,
	ja,
	jnp,
	jno,
	jns,
	loop,
	loopz,
	loopnz,
	jcxz,

	_int,
	int3,

	into,
	iret,

	clc,
	cmc,
	stc,
	cld,
	std,
	cli,
	sti,
	hlt,
	wait,
	esc,
	lock,
	segment,

	Count,
}

Inst_Lock    : u32 : 0x1
Inst_Rep     : u32 : 0x2
Inst_Segment : u32 : 0x4
Inst_Wide    : u32 : 0x8
Inst_Far     : u32 : 0x10

Register_Access :: struct {
	index: u32,
	offset: u32,
	count: u32,
}

Effective_Address_Term :: struct {
	register: Register_Access,
	scale: i32,
}

Effective_Address_Flag :: enum u32 {
	ExplicitSegment = 0x1,
}

Effective_Address_Expression :: struct {
	terms: [2]Effective_Address_Term,
	explicit_segment: u32,
	displacement: i32,
	flags: u32,
};

Immediate_RelativeJumpDisplacement : u32 : 0x1

Immediate :: struct {
	value: i32,
	flags: u32,
};

Operand_Type :: enum u32 {
	None,
	Register,
	Memory,
	Immediate,
};

Instruction_Operand :: struct {
	type: Operand_Type,
	using _ : struct #raw_union {
		address: Effective_Address_Expression,
		register: Register_Access,
		immediate: Immediate,
	},
}

Instruction :: struct {
	address: u32,
	size: u32,

	op: Operation_Type,
	flags: u32,

	operands: [2]Instruction_Operand,

	segment_override: u32,
};

Instruction_Bits_Usage :: enum u8 {
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

Instruction_Bits :: struct {
	usage: Instruction_Bits_Usage,
	bit_count: u8,
	shift: u8,
	value: u8,
};

Instruction_Encoding :: struct {
	op: Operation_Type,
	bits: [16]Instruction_Bits,
};

Instruction_Table :: struct {
	encodings: [^]Instruction_Encoding,
	encoding_count: u32,
	max_instruction_byte_count: u32,
};

@(default_calling_convention="c", link_prefix="Sim86_")
foreign sim86 {
	GetVersion :: proc() -> u32 ---
	Decode8086Instruction :: proc(source_size: u32, source: [^]u8, dest: [^]Instruction) ---
	RegisterNameFromOperand :: proc(reg_access: ^Register_Access) -> cstring ---
	Sim86_MnemonicFromOperationType :: proc(type: Operation_Type) -> cstring ---
	Get8086InstructionTable :: proc(dest: ^Instruction_Table) ---
}
