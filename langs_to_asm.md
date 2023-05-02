# How to view ASM output from your language

In Part 1 of the [Performance-Aware Programming course](https://www.computerenhance.com/p/table-of-contents), we learn to read assembly language through fun homework exercises where we simulate a few 8086 instructions. These assignments are designed to be completable in any programming language, so people can take the course in the language of their choice.

In Part 2, we learn to analyze compiler output and assess the performance of instruction sequences. This requires writing small snippets of code in a language that _can be compiled_. To allow people to take the course in as many programming languages as possible, I am collecting instructions on how to produce assembly-language listings in different languages.

Hopefully this will provide a growing quick-reference for people learning to read ASM to better understand the performance of their programs.

# Using Compiler Explorer

If it supports machine code output for your language, Compiler Explorer is often the simplest way to get a quick glance at the ASM for small programs. You cannot "step through" the ASM to see what it's doing like you can in a debugger, but it lets you see multiple compilations of the same program at once, which can be very useful.

* Go to [godbolt.org](https://godbolt.org).
* Select your language from the language drop-down.
* Select your compiler from the compiler drop-down. If you do not see a compiler drop-down (or if you'd like to add a second compiler), you can add a new compiler window by selecting `Add new... -> Compiler`
* If you are unsure what compiler options you should put in the `Compiler Options` box, consult the following table for recommendations:

| Language  | Compiler  | Typical Compiler Options |
| --------- | --------- | ------------------------ |
| C / C++   | MSVC      | -O2 |
| C / C++   | GCC       | -O3 |
| C / C++   | CLANG     | -O3 |
| C#        | .NET      | |
| ZIG       | ZIG       | -OReleaseFast / -OReleaseSmall / -OReleaseSafe |

Note that not all languages supported by Compiler Explorer provide _actual_ ASM output. Some - like Python - only provide interpreter bytecode output, which is not particularly useful. The table above includes only those languages for which Compiler Explorer can produce real ASM.

# Using a debugger

To see source code along with reasonable ASM when using a debugger, you must ensure that your executable has been built with _both_ debug information _and_ optimizations. If you are unsure what switches are required for this, the following table can get you started:

| Language  | Compiler  | Typical Compiler Options |
| --------- | --------- | ------------------------ |
| C / C++   | MSVC      | -Zi -O2 |
| C / C++   | GCC       | -g -O3 |
| C / C++   | CLANG     | -g -O3 |
| ZIG       | ZIG       | -femit-asm -OReleaseFast |

Once you have an executable that has debug information _and_ optimizations enabled, you can load the executable into a debugger and step through the ASM using the instructions below:

### Visual Studio

* If you did not build the executable inside Visual Studio to begin with, you can open an executable directly by selecting `Open -> Project/Solution...` from the `File` menu, then picking `Exe Project Files (*.exe)` from the file type drop-down. This will allow you to open any executable for debugging.
* Press `F11` or select `Step Into` from the `Debug` menu to begin debugging the program.
* If you are viewing source code only, right-click in the source code window and select `Go To Disassembly` from the context menu.
* To step through system or library ASM without Visual Studio warning you, make sure `Enable address-level debugging` and `Show disassembly if source is not available` are both checked from the `Options` dialog under the `Debug` menu (`Debugging -> General`).

### LLDB

If your binary has debugging symbols (see above), you can disassemble whole 
functions or stack frames.

* Start `lldb` and read the binary: `lldb -- <binary>` (e.g. `lldb -- test`)
* Switch to Intel disassembly flavor: `settings set target.x86-disassembly-flavor intel` (See the note below if you wonder what this means!).
* To disassemble a specific function, e.g. `main`: `disassemble -b -n main`
* Set a breakpoint and run to it, e.g. `b main` then `run`
* To disassemble the current function for the current stack frame: `disassemble -b`
* To display disassembly on every step/stepi: `settings set stop-disassembly-display always`
* To step by instruction use `stepi`, `nexti` (as apposed to `step`, `next`)

There are more options to the disassembler you may wish to use (see `help disassemble`), but the most interesting ones ones are:

* `-F intel` (also set by `target.x86-disassembly-flavor`) - use intel dialect.  This is not explicitly discussed in the course so far, but by default LLDB will use an ASM dialect known as AT&T style. For the purposes of this course, it's recommended to always specify `-F intel`. If you get a message about `Disassembler flavors are currently only supported for x86 and x86_64 targets`, you might be on an Apple Silicon Mac; see the note below.
* `-b` - show the instruction bytes. This shows the actual bytes as hex as well as the disassembly.
* `-n <name>` - disassemble function named `<name>`

***Note for Apple Silicon:*** If you are on an Apple Silicon Mac and following
this course, you might be confused by the disassembly output not matching the
x86_64 instructions. That's because Apple Silicon is an ARM procesor. SImply
pass `-arch x86_64` to the `clang` compiler to produce an Intel/x86_64 binary
and use the above instructions unchanged.

To avoid having to type `settings set ...` every time, you can put these
commands into a `.lldbinit` file in your home directory:

```
settings set target.x86-disassembly-flavor intel
settings set stop-disassembly-display always
```

# Dumping an executable to ASM

Several command line utilities are available 

### Dumpbin

* Install Visual Studio if you do not already have it installed.
* Open the "x64 Native Tools Command Prompt" from the Start Menu.
* To disassemble an executable (in this case, "test.exe") to an ASM listing file, use:
```
dumpbin /disasm test.exe > test_disassembly.asm
```

### otool

otool is a command-line utility which examines Mach-O files for macOS.

* Install Xcode or the Xcode Command Line Tools if you do not already have otool installed.
* To disassemble an executable (in this case, "test") to an ASM listing file, use:
```
otool -tV test > test_disassembly.asm
```
To demangle C++ symbols, also use c++filt (which is also installed with Xcode):
```
otool -tV test | c++filt > test_disassembly.asm
```

### objdump

objdump is a command-line utility which examines many different executable
files, including ELF for Linux.

There are two tools called objdump: the original objdump from GNU binutils and
LLVM's objdump. This section describes the objdump from GNU binutils, but these
instructions may work for LLVM's objdump too.

* Install GNU binutils if you do not already have objdump installed.
* To disassemble an executable (in this case, "test") to an ASM listing file, use:
```
objdump -d --demangle test > test_disassembly.asm
```

# If you'd like to contribute a new language or method...

... please submit a pull request! Note that in order to be included here, you must provide comprehensive instructions that allow someone to either step through or conveniently view the assembly language output of a program snippet.

\- [Casey](https://substack.com/profile/11696674-casey-muratori)
