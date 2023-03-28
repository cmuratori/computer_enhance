#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/sim86_lib_bindings.rs"));

use std::mem;

#[cfg(test)]
mod test {

    use super::*;

    #[test]
    fn decode_register_to_register_mov() {

        let disassembly: [u8;2] = [
            0b10001001, 0b11000100, // same in binary

            // Ob 100010 01 -> Opcode : Mov, D: false, W: true
            // 0b 11 000 100 -> Mod: 11, Reg: 000 R/M: 100 -> Reg AX, RM: SP
        ];

        let decoded_instruction = unsafe {
            let SourceSize = disassembly.len() as u32;
            let mut Decoded: instruction = mem::zeroed();

            let mut Source = disassembly[0];

            Sim86_Decode8086Instruction(SourceSize,
                                        &mut Source,
                                        &mut Decoded);
            Decoded
        };

        assert_eq!(decoded_instruction.Op, operation_type_Op_mov);

        // Operand 1 is properly parsed
        let operand1 = decoded_instruction.Operands[1];
        assert_eq!(operand1.Type, operand_type_Operand_Register);
        let operand1_register_access = unsafe { operand1.__bindgen_anon_1.Register  };
        assert_eq!(operand1_register_access.Index, 1);
        assert_eq!(operand1_register_access.Offset, 0);
        assert_eq!(operand1_register_access.Count, 2);

        // Operand 0 is not
        let operand0 = decoded_instruction.Operands[0];
        assert_eq!(operand0.Type, operand_type_Operand_Register);
        let operand0_register_access = unsafe { operand0.__bindgen_anon_1.Register };
        assert_eq!(operand0_register_access.Index, 5);
        assert_eq!(operand0_register_access.Offset, 0);
        assert_eq!(operand0_register_access.Count, 2);

    }

}

fn main() {
}
