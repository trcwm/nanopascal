/*

  Virtual machine type declarations for Micro Pascal VM
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#ifndef vmtypes_h
#define vmtypes_h

#include <stdint.h>
#include <vector>

namespace VM
{
    enum VMInstruction
    {
        VM_NOP      = 0,        // no operation (-)
        VM_LOAD     = 1,        // load local variable onto data stack (stack: 1 on)
        VM_STOREP   = 2,        // store local variable and pop off stack (stack: 1 off)
        VM_ADD      = 3,        // add two numbers (stack: 2 off, 1 on)
        VM_SUB      = 4,        // sub two numbers (stack: 2 off, 1 on)
        VM_MUL      = 5,        // mul two numbers (stack: 2 off, 1 on)
        VM_DIV      = 6,        // div two numbers (stack: 2 off, 1 on)
        VM_JMP      = 7,        // jump to address c
        VM_JZ       = 8,        // jump zero
        VM_JNZ      = 9,        // jump not zero
        VM_CEQ      = 10,       // compare equal (stack: 2 off, 1 on)
        VM_CNE      = 11,       // compare not equal (stack: 2 off, 1 on)
        VM_CL       = 12,       // compare less (unsigned) (stack: 2 off, 1 on)
        VM_CLE      = 13,       // compare less or equal (unsigned) (stack: 2 off, 1 on)
        VM_CG       = 14,       // compare greater (unsigned) (stack: 2 off, 1 on)
        VM_CGE      = 15,       // compare greater or equal (unsigned) (stack: 2 off, 1 on)
        VM_CALL     = 16,       // calls a function
        VM_RET      = 17,       // return from function
        VM_RESERVE  = 18,       // reserve n number of bytes on return stack
        VM_LIT      = 19,       // load a 16-bit literal on the stack
        VM_NEG      = 20,       // negate the top of stack (-)
        VM_NOT      = 21,       // binary inverse of top of stack (-)
        VM_INC      = 22,       // increment top of stack (-)
        VM_DEC      = 23,       // decrement top of stack (-)
        VM_WRITE    = 24,       // write the integer to the console (stack: 1 off)
        VM_LOADARG  = 25,       // load function/procedure argument (stack: 1 on)
        VM_LEAVE    = 26,       // clean up return stack ()
        VM_DONE     = 0xFF
    };

    void disassemble(const std::vector<uint8_t> &bytecode, bool bigEndian = false);

}   //namespace

#endif
