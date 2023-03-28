extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    // Build the sim86 library
    cc::Build::new()
        .file("../../sim86_lib.cpp")
        .compile("sim86_shared_lib");

    //
    // Tell cargo to look for shared libraries in the specified directory
    // NOTE(oht): not needed since we build it ouselves ?
    // println!("cargo:rustc-link-search=/path/to/lib");

    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    // println!("cargo:rustc-link-lib=bz2");
    // NOTE(pht): hoping it will find the lib itself ?
    println!("cargo:rustc-link-lib=sim86_shared_lib");

    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=shared/sim86_shared.hpp");

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("shared/sim86_shared.hpp")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("sim86_lib_bindings.rs"))
        .expect("Couldn't write bindings!");
}
