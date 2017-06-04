
#ifndef vm_h
#define vm_h

#include <stdint.h>
#include <stdbool.h>

#define DSTACKSIZE 256
#define RSTACKSIZE 256


typedef struct
{
    uint8_t  progmem[16384];      /* 16K program memory     */
    uint16_t dstack[DSTACKSIZE];  /* data stack             */
    uint16_t rstack[RSTACKSIZE];  /* return stack           */
    uint16_t pc;                  /* program counter        */
    uint16_t dsp;                 /* data stack pointer     */
    uint16_t rsp;                 /* return stack pointer   */
    uint16_t bp;                  /* base pointer           */
} vm_context_t;


typedef enum
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
    VM_RESERVE  = 18,       // reserve x number of words for local storage
    VM_LIT      = 19,       // load a 16-bit literal on the stack
    VM_NEG      = 20,       // negate the top of stack (-)
    VM_NOT      = 21,       // binary inverse of top of stack (-)
    VM_INC      = 22,       // increment top of stack (-)
    VM_DEC      = 23,       // decrement top of stack (-)
    VM_WRITE    = 24,       // write top of stack to console (stack: 1 off)
    VM_LOADARG  = 25,       // load function/procedure argument (stack: 1 on)
    VM_LEAVE    = 26,       // clean up return stack ()
    VM_DONE     = 0xFF
} vm_command_t;

void vm_init(vm_context_t *c);
void vm_load(vm_context_t *c, const uint8_t *bytecode, uint32_t bytes);
bool vm_loadfile(vm_context_t *c, const char *filename);

bool vm_execute(vm_context_t *c);

#endif
