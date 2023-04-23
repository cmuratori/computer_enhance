# Zig Wrapper for 8086 Simulator

## About

This wrapper implementation differs from others as it does not utilize any of
the precompiled libraries. It leverages Zig's interop with C (see links below),
to build the C++ shared library and then interface with it via the
`sim86_shared.h` C/C++ header file.

## Building and running

Tested Zig Version: `0.11.0-dev.2725+4374ce51b`

To run the test file:

```shell
$ pwd # Should be perfaware/sim86/shared/contrib_zig
$ zig build test
```

To include this in your own Zig project:

1. Download this repo into your project. My suggestions is to add this repo as a
   git submodule of your project repo: `git submodule add https://github.com/cmuratori/computer_enhance`.
   You can instantiate this submodule in directory with a name like `vendor` or
   `thirdparty`.
2. Call `addStaticLibrary` from your `build.zig` in a similar fashion to `lib`
   in this directory's `build.zig`. If you used `vendor` as your directory name for
   the downloaded repo your `addIncludePath`, `addCSourceFile`, and `installHeader`
   may look like this:

```zig
    lib.addCSourceFile("vendor/computer_enhance/perfaware/sim86/sim86_lib.cpp", &.{});
    lib.addIncludePath("vendor/computer_enhance/perfaware/sim86/shared");
    lib.installHeader("vendor/computer_enhance/perfaware/sim86/shared/sim86_shared.h", "sim86_shared.h");
```

When the Zig package manager is ready, this should be easily downloadable.

## Links

- [C Interop](https://ziglang.org/learn/overview/#integration-with-c-libraries-without-ffibindings)
