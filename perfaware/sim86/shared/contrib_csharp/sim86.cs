using System.Diagnostics;
using System.Runtime.InteropServices;

#pragma warning disable CS8603

static class Sim86
{
    public enum OperationType : uint
    {
        None,
        mov,
        push,
        pop,
        xchg,
        @in,
        @out,
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
        @int,
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
        @lock,
        segment,
    }

    [Flags]
    public enum InstructionFlag : uint
    {
        Lock = 0x1,
        Rep = 0x2,
        Segment = 0x4,
        Wide = 0x8,
        Far = 0x10,
    };

    [Flags]
    public enum EffectiveAddressFlag : uint
    {
        ExplicitSegment = 0x1,
    };

    [Flags]
    public enum ImmediateFlag : uint
    {
        RelativeJumpDisplacement = 0x1,
    };

    public enum InstructionBitsUsage : byte
    {
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

    [StructLayout(LayoutKind.Sequential)]
    public struct RegisterAccess
    {
        public uint Index;
        public uint Offset;
        public uint Count;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct EffectiveAddressTerm
    {
        public RegisterAccess Register;
        public int Scale;
    };

    public struct EffectiveAddressExpression
    {
        public EffectiveAddressTerm[] Term;
        public uint ExplicitSegment;
        public int Displacement;
        public EffectiveAddressFlag Flags;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Immediate
    {
        public int Value;
        public ImmediateFlag Flags;
    };

    public struct Instruction
    {
        public uint Address;
        public int Size;
        public OperationType Op;
        public InstructionFlag Flags;
        public object[] Operands;
        public uint SegmentOverride;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct InstructionBits
    {
        public InstructionBitsUsage Usage;
        public byte BitCount;
        public byte Shift;
        public byte Value;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct InstructionEncoding
    {
        public OperationType Op;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public InstructionBits[] Bits;
    };

    public struct InstructionTable
    {
        public InstructionEncoding[] Encodings;
        public uint MaxInstructionByteCount;
    };

    private static class Native
    {
        const string dll = "sim86_shared_debug";

        public enum OperandType : uint
        {
            None,
            Register,
            Memory,
            Immediate,
        };

        [StructLayout(LayoutKind.Sequential)]
        public struct EffectiveAddressExpression
        {
            public EffectiveAddressTerm Term0;
            public EffectiveAddressTerm Term1;
            public uint ExplicitSegment;
            public int Displacement;
            public EffectiveAddressFlag Flags;
        };

        [StructLayout(LayoutKind.Explicit)]
        public struct InstructionOperand
        {
            [FieldOffset(0)]
            public OperandType OpType;

            [FieldOffset(4)]
            public EffectiveAddressExpression Address;

            [FieldOffset(4)]
            public RegisterAccess Register;

            [FieldOffset(4)]
            public Immediate Immediate;
        };

        [StructLayout(LayoutKind.Sequential)]
        public struct Instruction
        {
            public uint Address;
            public uint Size;
            public OperationType Op;
            public InstructionFlag Flags;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
            public InstructionOperand[] Operands;
            public uint SegmentOverride;
        };

        [StructLayout(LayoutKind.Sequential)]
        public struct InstructionTable
        {
            public IntPtr Encodings;
            public int EncodingCount;
            public uint MaxInstructionByteCount;
        }

        [DllImport(dll)]
        public static extern uint Sim86_GetVersion();

        [DllImport(dll)]
        public static extern void Sim86_Decode8086Instruction(uint SourceSize, [In] ref byte Source, out Instruction Dest);

        [DllImport(dll)]
        public static extern IntPtr Sim86_RegisterNameFromOperand([In] ref RegisterAccess RegAccess);

        [DllImport(dll)]
        public static extern IntPtr Sim86_MnemonicFromOperationType(OperationType Type);

        [DllImport(dll)]
        public static extern void Sim86_Get8086InstructionTable(out InstructionTable Dest);
    }

    public const int Version = 3;

    public static uint GetVersion()
    {
        return Native.Sim86_GetVersion();
    }

    public static Instruction Decode8086Instruction(Span<byte> Source)
    {
        Native.Instruction NativeInstruction;
        Native.Sim86_Decode8086Instruction((uint)Source.Length, ref MemoryMarshal.AsRef<byte>(Source), out NativeInstruction);

        return new Instruction()
        {
            Address = NativeInstruction.Address,
            Size = (int)NativeInstruction.Size,
            Op = NativeInstruction.Op,
            Flags = NativeInstruction.Flags,
            Operands = NativeInstruction.Operands
                .Where(o => o.OpType != Native.OperandType.None)
                .Select<Native.InstructionOperand, object>(o =>
                {
                    if (o.OpType == Native.OperandType.Register)
                    {
                        return o.Register;
                    }
                    else if (o.OpType == Native.OperandType.Memory)
                    {
                        return new EffectiveAddressExpression()
                        {
                            Term = new[] { o.Address.Term0, o.Address.Term1 },
                            ExplicitSegment = o.Address.ExplicitSegment,
                            Displacement = o.Address.Displacement,
                            Flags = o.Address.Flags,
                        };
                    }
                    else
                    {
                        Debug.Assert(o.OpType == Native.OperandType.Immediate);
                        return o.Immediate;
                    }
                })
                .ToArray(),
            SegmentOverride = NativeInstruction.SegmentOverride,
        };
    }

    public static string RegisterNameFromOperand(RegisterAccess RegAccess)
    {
        return Marshal.PtrToStringAnsi(Native.Sim86_RegisterNameFromOperand(ref RegAccess));
    }

    public static string MnemonicFromOperationType(OperationType Type)
    {
        return Marshal.PtrToStringAnsi(Native.Sim86_MnemonicFromOperationType(Type));
    }

    public static InstructionTable Get8086InstructionTable()
    {
        Native.InstructionTable NativeTable;
        Native.Sim86_Get8086InstructionTable(out NativeTable);

        return new InstructionTable()
        {
            Encodings = Enumerable
                .Range(0, NativeTable.EncodingCount)
                .Select(n => Marshal.PtrToStructure<InstructionEncoding>(NativeTable.Encodings + n * Marshal.SizeOf<InstructionEncoding>()))
                .ToArray(),
            MaxInstructionByteCount = NativeTable.MaxInstructionByteCount,
        };
    }
}
