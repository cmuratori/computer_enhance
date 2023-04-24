// place "sim86_shared_debug.dll" next to all of these three files (sim86.cs, sim86_test.cs, sim86.csproj)
// then run "dotnet.exe run"

internal class Program
{
    static int[] Registers = new int[8];
    private static void Main(string[] args)
    {
        var ExampleDisassembly = File.ReadAllBytes("/home/ethan/repos/computer_enhance/perfaware/part1/listing_0043_immediate_movs");
        Console.WriteLine($"Sim86 Version: {Sim86.GetVersion()}");

        var Table = Sim86.Get8086InstructionTable();
        Console.WriteLine($"8086 Instruction Instruction Encoding Count: {Table.Encodings.Length}");

        var Offset = 0;
        while (Offset < ExampleDisassembly.Length)
        {
            var Decoded = Sim86.Decode8086Instruction(ExampleDisassembly.AsSpan().Slice(Offset));
            Offset += Decoded.Size;
            if (Decoded.Op != Sim86.OperationType.None)
            {
                if (Decoded.Op == Sim86.OperationType.mov)
                {
                    if (Decoded.Operands[0] is Sim86.RegisterAccess reg)
                    {
                        var registerName = Sim86.RegisterNameFromOperand(reg);
                        var registerId = (RegisterId)Enum.Parse(typeof(RegisterId), registerName);
                        if (Decoded.Operands[1] is Sim86.Immediate imm)
                        {
                            var register = Registers[(int)registerId];
                            Console.WriteLine($"{Decoded.Op} {registerName}, {imm.Value} ; {registerName}:0x{register.ToString("X")}->0x{imm.Value.ToString("X")}");
                            Registers[(int)registerId] = imm.Value;
                        }
                    }
                }

            }
            else
            {
                Console.WriteLine("unrecognized instruction");
                break;
            }
        }

        for (var i = 0; i < Registers.Length; i++)
        {
            Console.WriteLine($"{(RegisterId)i}: {Registers[i]}");
        }
    }
}

enum RegisterId
{
    ax,
    bx,
    cx,
    dx,
    sp,
    bp,
    si,
    di
}