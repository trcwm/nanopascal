# Nano Pascal
A Tiny Pascal compiler with a virtual machine target to bootstrap systems.

## Current state of the project
About as useful as a bucket of sand in the desert.

## Introduction
Nano Pascal cross-compiler was developed to be able to write programs for an HD6309-based 8-bit computer. At the time of writing, the only way to take advantage of the HD6309 capabilities was to write directly in assembly langauge.

The compiler is not aimed to produce the fastest code or smallest code, but to allow to bring up a system in a short amount of time. This is accomplished by generating byte code for a virtual machine instead of native HD6309 code. A small hand-crafted virtual machine executes the byte code on the 8-bit CPU.

Porting the compiler to another CPU is straightforward. Only the virtual machine needs to be implemented on the target CPU; the byte code remains the same.

Nano Pascal implements a subset of the Pascal langauge. For instance, it does not support floating-point numbers or pointers.

## Building the compiler
The compiler should compile on Windows, Linux and OSX. 

Dependencies:
* Ninja build
* CMake
* GCC or Clang

## Ready-made binaries
At this time, there are no ready-made binaries available.
