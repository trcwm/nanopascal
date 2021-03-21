/*

    p-code machine based on Niklaus Wirth's VM

    byte 1  : operand code (lower nibble), level high nibble
    byte 2,3: n - numeric constant or
              a - address

    the machine is little-endian

*/

#pragma once 
#include <stdint.h>

typedef struct
{
    uint8_t  *mem;      /* program memory               */
    int16_t  *dstack;   /* data stack memory            */
    uint16_t pc;        /* program counter     (mem)    */
    uint16_t t;         /* stack pointer/index (dstack) */
    uint16_t b;         /* base pointer/index  (dstack) */
    size_t   inscount;  /* number of instructions executed */
    uint16_t memsize;   /* number of bytes in mem buffer */
} vm_context_t;


typedef enum
{
    VM_LIT      = 0,        // load literal constant 0,n
    VM_OPR      = 1,        // arithmetic or logical operation 0,n
    VM_LOD      = 2,        // load variable v,d
    VM_STO      = 3,        // store variable v,d
    VM_CAL      = 4,        // call procedure or function v,a
    VM_INT      = 5,        // increment stack pointer 0,n
    VM_JMP      = 6,        // unconditional jump 0,a
    VM_JPC      = 7,        // jump if false (tos = 0) 0,a
    VM_HALT
} opcode_t; // lower nibble code

// functions of the VM_OPR code (stored in the 16 bit immediate)
typedef enum
{
    OPR_RET     = 0,
    OPR_NEG     = 1,
    OPR_ADD     = 2,
    OPR_SUB     = 3,
    OPR_MUL     = 4,
    OPR_DIV     = 5,
    OPR_ODD     = 6,
    OPR_NULL    = 7,    // ??
    OPR_EQ      = 8,
    OPR_NEQ     = 9,
    OPR_LESS    = 10,
    OPR_LEQ     = 11,
    OPR_GREATER = 12,
    OPR_GEQ     = 13,
    OPR_SHR     = 14,
    OPR_SHL     = 15,
    OPR_SAR     = 16,
    OPR_OUTCHAR = 17,
    OPR_OUTINT  = 18,
    OPR_INCHAR  = 19,
    OPR_ININT   = 20
} opr_t;

#pragma pack(push,1)
typedef struct
{
    uint8_t     opcode; // lower nibble is opcode_t, upper is level for procedures/functions
    uint16_t    opt16;  // optional 16-bit payload (address or literal)
} instruction_t;
#pragma pack(pop)
