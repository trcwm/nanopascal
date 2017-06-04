
#include <memory.h>
#include <stdio.h>
#include "vm.h"

/** return get a 16-bit word from a memory address.
    note: This function behaves differently
          depending on the CPU's endianness!
          Most CPUs are little endian, but
          the HD6309 is not!
*/
uint16_t getWordFromPMEM(vm_context_t *c, uint16_t pc)
{
    uint16_t *ptr = (uint16_t*)(c->progmem+pc);
    return *ptr;
}

/** get a local variable from rstack memory */
uint16_t getLocal(vm_context_t *c, uint16_t offset)
{
    uint16_t *ptr = c->rstack + c->bp - offset;
    return *ptr;
}

/** get a local variable from rstack memory

    stack layout is as follows:

    0 ARG 1
    2 ARG 2
    4 ARG 3
    6 RETURN ADDRESS
    8 PREV BP STORED HERE
   10

   -10 <-- new BP points here
   -8  PREV BP
   -6  RET ADDRESS
   -4  ARG 3
   -2  ARG 2
    0  ARG 1

   LOADARG #0 should load ARG 3
   LOADARG #2 should load ARG 2 etc.
*/

uint16_t getArgument(vm_context_t *c, uint16_t offset)
{
    uint16_t *ptr = c->rstack + c->bp + offset + 6;
    return *ptr;
}

/** store a local variable in rstack memory */
void storeLocal(vm_context_t *c, uint16_t offset, uint16_t val)
{
    uint16_t *ptr = c->rstack + c->bp - offset;
    ptr[0] = val;
}

void vm_init(vm_context_t *c)
{
    c->pc  = 0;
    c->dsp = DSTACKSIZE-1;
    c->rsp = RSTACKSIZE-1;
    c->bp  = 0;
}


void vm_load(vm_context_t *c, const uint8_t *bytecode, uint32_t bytes)
{
    memcpy(c->progmem, bytecode, bytes);
    vm_init(c);
}


bool vm_loadfile(vm_context_t *c, const char *filename)
{
    FILE *fin = fopen(filename, "rb");
    if (fin == NULL)
    {
        return false;
    }

    vm_init(c);
    while(!feof(fin))
    {
        c->progmem[c->pc++] = fgetc(fin);
    }
    fclose(fin);

    printf("Loaded %d bytes\n", c->pc);

    c->pc = 0; // program reset
    return true;
}

bool vm_execute(vm_context_t *c)
{
    switch(c->progmem[c->pc])
    {
    case VM_ADD:
        c->dsp++;
        c->dstack[c->dsp+1] += c->dstack[c->dsp];
        c->pc++;
        break;
    case VM_SUB:
        c->dsp++;
        c->dstack[c->dsp+1] -= c->dstack[c->dsp];
        c->pc++;
        break;
    case VM_DIV:
        c->dsp++;
        c->dstack[c->dsp+1] /= c->dstack[c->dsp];
        c->pc++;
        break;
    case VM_MUL:
        c->dsp++;
        c->dstack[c->dsp+1] *= c->dstack[c->dsp];
        c->pc++;
        break;
    case VM_INC:
        c->dstack[c->dsp+1]++;
        c->pc++;
        break;
    case VM_DEC:
        c->dstack[c->dsp+1]--;
        c->pc++;
        break;
    case VM_DONE:
        return false;
        break;
    case VM_NEG:
        c->dstack[c->dsp+1] = (~c->dstack[c->dsp+1])+1;
        c->pc++;
        break;
    case VM_NOT:
        c->dstack[c->dsp+1] = ~c->dstack[c->dsp+1];
        c->pc++;
        break;
    case VM_CL:
        c->dsp++;
        if ((c->dstack[c->dsp+1]) < (c->dstack[c->dsp]))
        {
            c->dstack[c->dsp+1] = 1;
        }
        else
        {
            c->dstack[c->dsp+1] = 0;
        }
        c->pc++;
        break;
    case VM_CLE:
        c->dsp++;
        if ((c->dstack[c->dsp+1]) <= (c->dstack[c->dsp]))
        {
            c->dstack[c->dsp+1] = 1;
        }
        else
        {
            c->dstack[c->dsp+1] = 0;
        }
        c->pc++;
        break;
    case VM_CG:
        c->dsp++;
        if ((c->dstack[c->dsp+1]) > (c->dstack[c->dsp]))
        {
            c->dstack[c->dsp+1] = 1;
        }
        else
        {
            c->dstack[c->dsp+1] = 0;
        }
        c->pc++;
        break;
    case VM_CGE:
        c->dsp++;
        if ((c->dstack[c->dsp+1]) >= (c->dstack[c->dsp]))
        {
            c->dstack[c->dsp+1] = 1;
        }
        else
        {
            c->dstack[c->dsp+1] = 0;
        }
        c->pc++;
        break;
    case VM_JMP:
        c->pc = getWordFromPMEM(c,c->pc+1);
        break;
    case VM_LOAD:
        c->dstack[c->dsp] = getLocal(c,getWordFromPMEM(c,c->pc+1) >> 1);
        c->dsp--;
        c->pc+=3;
        break;
    case VM_STOREP:
        c->dsp++;
        storeLocal(c, getWordFromPMEM(c,c->pc+1) >> 1, c->dstack[c->dsp]);
        c->pc+=3;
        break;
    case VM_RESERVE:
        /** this essentially creates a stack frame */
        c->dstack[c->dsp--] = c->bp;  // save old BP!
        c->bp = c->rsp;

        // note: rstack is uint16_t but
        // reserve as bytes as size
        // so, we divide by two and
        // reserve one additional uint16_t to
        // be safe!

        c->dsp -= getWordFromPMEM(c,c->pc+1) >> 1;
        c->pc+=3;
        break;
    case VM_LEAVE:
        /** destroy the stack frame */
        c->dsp = c->bp;                 // recover prev stack pointer
        c->bp = c->rstack[++c->rsp];    // recover prev bp
        c->pc += 3;
        break;
    case VM_CALL:
        /** call a function */
        c->dstack[c->dsp--] = c->pc+3;
        c->pc = getWordFromPMEM(c,c->pc+1); // call address
        break;
    case VM_RET:
        /** return from function */
        c->pc = c->dstack[++c->dsp];
        break;
    case VM_LOADARG:
        c->dstack[c->dsp] = getArgument(c,getWordFromPMEM(c,c->pc+1) >> 1);
        c->dsp--;
        c->pc+=3;
        break;
    case VM_LIT:
        c->dstack[c->dsp] = getWordFromPMEM(c,c->pc+1);
        c->dsp--;
        c->pc+=3;
        break;
    case VM_JNZ:
        c->dsp++;
        if (c->dstack[c->dsp] == 0)
        {
            // false, no jump
            c->pc += 3;
        }
        else
        {
            // true, jump
            c->pc = getWordFromPMEM(c,c->pc+1);
        }
        break;
    case VM_JZ:
        c->dsp++;
        if (c->dstack[c->dsp] != 0)
        {
            // false, no jump
            c->pc += 3;
        }
        else
        {
            // true, jump
            c->pc = getWordFromPMEM(c,c->pc+1);
        }
        break;
    case VM_WRITE:
        c->dsp++;
        printf("%d\n", c->dstack[c->dsp]);
        c->pc++;
        break;
    default:
        printf("Unknown instruction at address %04X with code %02X -- aborting!\n", c->pc, c->progmem[c->pc]);
        return false;
        break;
    }

    return true;
}
