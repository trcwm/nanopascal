/*

    p-code machine based on Niklaus Wirth's VM

    byte 1  : operand code
    byte 2  : v   - static level difference
              c   - condition code, 
              255 - absolute addressing, 
              unused
    byte 3,4: n - numeric constant,
              a - address

    the machine is little-endian

*/

#ifndef vm_h
#define vm_h

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t  *mem;      /* program memory               */
    int16_t  *dstack;   /* data stack memory            */
    uint16_t pc;        /* program counter     (mem)    */
    uint16_t t;         /* stack pointer/index (dstack) */
    uint16_t b;         /* base pointer/index  (dstack) */
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
    VM_JPC      = 7         // conditional jump 0,a
} opcode_t;

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
    OPR_READ    = 14,
    OPR_WRITE   = 15
} opr_t;

#pragma pack(push,1)
typedef struct
{
    uint8_t     opcode; // lower nibble is opcode_t, upper is level for procedures/functions
    uint16_t    opt16;  // optional 16-bit payload (address or literal)
} instruction_t;
#pragma pack(pop)

void vm_init(vm_context_t *c, uint8_t *memptr);
void vm_free(vm_context_t *c);

void vm_load(vm_context_t *c, const uint8_t *bytecode, uint32_t bytes);
bool vm_loadfile(vm_context_t *c, const char *filename);

bool vm_execute(vm_context_t *c);

#endif
