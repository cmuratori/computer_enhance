# 8086 Simulator Reference

This repository is example code for homework assignments from the [Performance Aware Programming course](https://www.computerenhance.com/p/table-of-contents). Programmers enrolled in the course are greatly encouraged to write their own code for the homework assignments rather than using this code.

However, should you wish to use this code as a starting point for future homework, or to check your work, please make sure you _first select the appropriate github tag_ before doing so. The state of the simulator will update as the homework progresses, so it is important to make sure you are looking at source code that matches where you are in the homework.

### Tags:

**Part1_0_SlowDecode**: No simulation, just decoding. The decoding works directly off a transcription of the table from the 8086 manual, with no attempt to accelerate it, so decoding is quite slow (compared to what we could expect if we used a lookup). After instructions are decoded, they are printed to the console in Intel ASM syntax. This decoder is only tested to guarantee binary-exact reassembly up through [ASM listing 42](../part1/listing_0042_completionist_decode.asm), and may fail on 8086 machine code not covered by those tests.
