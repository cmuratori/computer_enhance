### Welcome

CPU: the thing that has to do all of the work

Our programs are instructions that go into the cpu.
How long does the cpu take to process those instructions?

How improve performance. (Reduce the time it takes for the program to complete)
Either:

1. Reduce the _number_ of instructions
2. Change the _speed_ of the instructions. Some instructions finish in one clock cycle. Other instructions have work that takes more to finish, perhaps 100s. In other words, change which instructions run.

How did programs get 1000x slower? Programmers used to be aware which instructions a cpu would have to do in order to run a program.

CPUs got more complicated. This made it less likely that programmers knew which instructions were used, since there were so many of them.

So, we need to start thinking not just in terms of our source language. Say, Java for example. but also think about what our code becomes in between until the CPU starts to execute it. For Java, this is the JIT compiler, I think.

It's what the source code turns _into_ that determines its speed.

We should keep this in mind when we program so to pause before writing something that even the JIT will be unable to find a good translation for.

Mental model: The Java Virtual Machine takes Java source code and translates it into bytecode. What happens from here, I'm not sure.
