#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/sim86_lib_bindings.rs"));

#[cfg(test)]
mod test {

    use super::*;

    #[test]
    fn decode_register_to_register_mov() {

        let disassembly: [u8;2] = [
            0b10001001, 0b11000100, // mov sp,ax; in binary, unless I'm mistaken

            // Ob 100010 01 -> Opcode : Mov, D: false, W: true
            // 0b 11 000 100 -> Mod: 11, Reg: 000 R/M: 100 -> Reg AX, RM: SP
        ];

        let mut Decoded = std::mem::MaybeUninit::uninit();
        let Source_ptr = disassembly.as_ptr() as *mut u8;
        // let mut Source = disassembly[0];

        let decoded_instruction : instruction = unsafe {
            let SourceSize = disassembly.len() as u32;

            Sim86_Decode8086Instruction(SourceSize,
                                        // &mut Source,
                                        Source_ptr,
                                        Decoded.as_mut_ptr());
            Decoded.assume_init()
        };

        assert_eq!(decoded_instruction.Op, operation_type_Op_mov);

        // Operand 1 is properly parsed
        let operand1 = decoded_instruction.Operands[1];
        assert_eq!(operand1.Type, operand_type_Operand_Register);
        let operand1_register_access = unsafe { operand1.__bindgen_anon_1.Register  };
        assert_eq!(operand1_register_access.Index, 1);
        assert_eq!(operand1_register_access.Offset, 0);
        assert_eq!(operand1_register_access.Count, 2);

        // Operand 0 is not properly parsed, for some reason, it's considered as a Memory...
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
