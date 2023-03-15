# 8086 Simulator Reference

This repository is example code for homework assignments from the [Performance Aware Programming course](https://www.computerenhance.com/p/table-of-contents). Programmers enrolled in the course are greatly encouraged to write their own code for the homework assignments rather than using this code.

However, should you wish to use this code as a starting point for future homework, or to check your work, please make sure you _first select the appropriate github tag_ before doing so. The state of the simulator will update as the homework progresses, so it is important to make sure you are looking at source code that matches where you are in the homework.

### Tags:

**Part1_0_SlowDecode**: No simulation, just decoding. The decoding works directly off a transcription of the table from the 8086 manual, with no attempt to accelerate it, so decoding is quite slow (compared to what we could expect if we used a lookup). After instructions are decoded, they are printed to the console in Intel ASM syntax. This decoder is only tested to guarantee binary-exact reassembly up through [ASM listing 42](../part1/listing_0042_completionist_decode.asm), and may fail on 8086 machine code not covered by those tests.

### Building:

The code is structured such that compiling `sim86.cpp` with any compiler should produce a usable console program, no build tools necessary. For example, to build the program with MSVC you would do:

```
cl sim86.cpp
```

It should work similarly with any C++ compiler. As an illustration, a `build.bat` file is provided that will make a build directory and build debug and release versions of the code with both MSVC and CLANG. However, there is nothing special about this batch file, it just compiles the file as above using some default switches (such as -O3 or -g).

### Running:

Once you have built an executable, you can run it by providing an 8086 machine code file, such as [this test file](../part1/listing_0042_completionist_decode):

```
sim86 listing_0042_completionist_decode
```

Assuming everything is working properly, it will print a disassembly of the machine code to the command line.

\- Casey
