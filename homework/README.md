## What this is

Repo of my homework from the course `Computer, enhance!`.

I have tried to complete the tasks in at least two different languages. The goal is to add more as time goes by, in order to have more samples to profile and compare timings from.

### 1: Instruction stream encoding/decoding

First make a binary instruction stream in `.asm`. And then decode it. For the decoding we write a program in our chosen language that can read binary files and disassemble them.

From Table 4-12 in [8086 Family User's Manual - Page 165](https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf#page=165) we know that a register-to-register copy in binary is `1 0 0 0 1 0 d w`. The first 6 bits specifies the opcode, here it signifies the "mov" operation.

The D (direction) field sets the direction of the mov. 1/0 is to-from destination register. Since we are moving _from_ source register _to_ destination register, our value becomes `D=1`.

The W field determines if the operation is a "byte-" (8-bit) or a "word-" (16 bit) operation. In our case, since we are copying the whole register (all 16 bits), `W=1`

The second byte of the instruction divided into the MOD (2 bits), REG (3 bits), and R/M(3 bits) -operands.

The MOD field sets whether both operands are in memory (`0 0`), both are registers (`1 1`), or in between.

So, until now we have `1 0 0 0 1 0 0 1 | `.
And in our second byte, we have `1 1 0 1 1 0 0 1`, the `0 1 1` being the REG code for BX, and `0 0 1` specifying the destination register (CX).

Together, that gives `1 0 0 0 1 0 0 1 | 1 1 0 1 1 0 0 1`, which is `89 D9` in hexadecimal.

For the second operation:

```
mov ch, ah
```

We will do another manual decode to prove that we have understood the concepts from the manual.

The opcode would be `1 0 0 0 1 0 0 0`, with the W field set to `0` since we are only copying the first 8 bits in the register.

Then we have `1 1 1 0 0 1 0 1` with `0 0 1` identifying `ah` and `1 0 1` identifying `ch` respectively.

Together, this give `1 0 0 0 1 0 0 0 | 1 1 1 0 0 1 0 1`, which is `88 E5` in hex.
