#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::mem::MaybeUninit;
use std::{borrow::Cow, ffi::CStr};

include!(concat!(env!("OUT_DIR"), "/sim86_shared.rs"));

pub fn get_version() -> u32 {
    unsafe { Sim86_GetVersion() }
}

pub fn get_8086_instruction_table() -> instruction_table {
    let mut table = MaybeUninit::uninit();

    unsafe {
        Sim86_Get8086InstructionTable(table.as_mut_ptr());
        table.assume_init()
    }
}

pub fn decode_8086_instruction(source: &[u8]) -> Option<instruction> {
    // We know for sure that the call to decode the instruction isn't mutating
    // the slice, so casting away the const to the same memory region should
    // be safe
    let mut_ptr = source.as_ptr() as *mut u8;

    let mut decoded_uninitialised = MaybeUninit::uninit();
    let decoded = unsafe {
        Sim86_Decode8086Instruction(
            source.len() as u32,
            mut_ptr,
            decoded_uninitialised.as_mut_ptr(),
        );

        decoded_uninitialised.assume_init()
    };

    if decoded.Op != operation_type_Op_None {
        Some(decoded)
    } else {
        None
    }
}

pub fn mnemonic_from_operation_type(op: operation_type) -> Cow<'static, str> {
    unsafe { CStr::from_ptr(Sim86_MnemonicFromOperationType(op)).to_string_lossy() }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn version_match_with_shared() {
        let version = get_version();
        assert_eq!(version, SIM86_VERSION);
    }
}
