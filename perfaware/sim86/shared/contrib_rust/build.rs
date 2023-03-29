use std::path::PathBuf;
use std::{env, fs};

fn main() {
    let lib_path = PathBuf::from("../");

    let header_file = lib_path.join("sim86_shared.h");

    println!("cargo:rustc-link-search={}", lib_path.display());
    println!("cargo:rustc-link-lib=sim86_shared_debug");
    println!("cargo:rerun-if-changed={}", header_file.display());

    let bindings = bindgen::Builder::default()
        .header("../sim86_shared.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Failed to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("sim86_shared.rs"))
        .expect("Failed to write bindings.rs");

    // NOTE:(rob) If we're running on windows then we need the DLL to be somewhere
    // we can find it, so we just place it in the root of the `contrib_rust` dir.
    // Does this need to be done for other platforms?

    if cfg!(target_os = "windows") {
        let shared_lib_path = lib_path.join("sim86_shared_debug.dll");
        let dest_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
        fs::copy(shared_lib_path, dest_path.join("sim86_shared_debug.dll"))
            .expect("Failed to copy the library to `contrib_rust/`");
    }
}
