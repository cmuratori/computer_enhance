package main

import "sim86"
import "core:fmt"

test_file := #load("../../../part1/listing_0042_completionist_decode")

main :: proc() {
	fmt.println("Sim86 Version:", sim86.GetVersion())

	table: sim86.Instruction_Table
	sim86.Get8086InstructionTable(&table)
	fmt.println("8086 Instruction Encoding Count:", table.encoding_count)

	offset: int = 0
	for offset < len(test_file) {
		decoded: sim86.Instruction
		sim86.Decode8086Instruction(u32(len(test_file) - offset), &test_file[offset], &decoded)
		if decoded.op != .None {
			offset += cast(int)decoded.size
		} else {
			fmt.println("Unrecognized instruction")
			break
		}
	}
}