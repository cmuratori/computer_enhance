const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addStaticLibrary(.{
        .name = "contrib_zig",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "src/sim86.zig" },
        .target = target,
        .optimize = optimize,
    });

    // NOTE(jorge): In Zig we have to tell the above Zig file where our included
    // header is. We also have to build the sim86 library file since that's
    // where the implementation lives. We link libcpp, because the
    // implementation is done in C++. Finally, we add sim86_shared.h as an
    // include path that any Zig file (including our test file) can use
    //
    // In "production" Zig, you wouldn't use relatives paths. You'd add the
    // sim86 library files to a `vendor` or `third_party` directory and source
    // from there.
    lib.addIncludePath("..");
    lib.addCSourceFile("../../sim86_lib.cpp", &.{});
    lib.linkLibCpp();
    lib.installHeader("../sim86_shared.h", "sim86_shared.h");

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(lib);

    // Creates a step for unit testing.
    const main_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/sim86_test.zig" },
        .target = target,
        .optimize = optimize,
    });
    main_tests.linkLibrary(lib);

    const run_main_tests = b.addRunArtifact(main_tests);

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build test`
    // This will evaluate the `test` step rather than the default, which is "install".
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_main_tests.step);
}
