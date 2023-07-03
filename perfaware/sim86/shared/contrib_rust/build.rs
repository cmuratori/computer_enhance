use std::path::PathBuf;
use std::env;

fn main() {
    let lib_path = PathBuf::from("../");

    let header_file = lib_path.join("sim86_shared.h");
    println!("cargo:rustc-link-lib=sim86_shared");

    println!("cargo:rerun-if-changed={}", header_file.display());

    // Compile the library ourselves so we don't need to rely on the existing platform specific binaries
    let mut build = cc::Build::new();

    if cfg!(target_os = "linux") {
        build
            .flag("-std=c++17")
            .flag("-Wno-missing-field-initializers")
            .flag("-Wno-unused-function");
    } else {
        build.flag("/std:c++17");
    }

    build
        .cpp(true)
        .file(lib_path.join("../sim86_lib.cpp"))
        .compile("sim86_shared");

    let bindings = bindgen::Builder::default()
        .clang_arg("--std=c++17")
        .clang_arg("-x")
        .clang_arg("c++")
        .header("../sim86_shared.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Failed to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("sim86_shared.rs"))
        .expect("Failed to write bindings.rs");
}
