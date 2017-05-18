# Micro-Pascal
Pascal cross-compiler for small 8-bit systems.

# Introduction
Micro Pascal cross-compiler was developed to be able to write programs for an HD6309-based 8-bit computer. At the time of writing, the only way to take advantage of the HD6309 capabilities was to write directly in assembly langauge.

The compiler is not aimed to produce the fastest code or smallest code, but to allow to bring up a system in a short amount of time. This is accomplished by generating byte code for a virtual machine instead of native HD6309 code. A small hand-crafted virtual machine executes the byte code on the 8-bit CPU.

Porting the compiler to another CPU is straightforward. Only the virtual machine needs to be implemented on the target CPU; the byte code remains the same.

Micro Pascal implements a subset of the Pascal langauge. For instance, it does not support floating-point numbers or pointers.

# Implementation
A tokenizer tokenizes the source code into an array of tokens. The parser takes the tokens and checks the grammar of program. At the same time, it produces an abstract syntax tree.

# Building the cross-compiler
The cross-compiler should compile on Windows, Linux and OSX. To build the software, install QtCreator and Qt5.x on your platform of choice. Using QtCreator, open the .pro project file. It will probably ask you to configure the project for your compiler. When done, select Build All from the menu.

# Ready-made binaries
At this time, there are no ready-made binaries available.