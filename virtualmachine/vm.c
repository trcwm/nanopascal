
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

void vm_init(vm_context_t *c, uint8_t *memptr, uint16_t memsize)
{
    c->dstack  = calloc(16384, sizeof(uint16_t));
    c->mem     = memptr;
    c->memsize = memsize;
    c->t  = 0;
    c->b  = 1;
    c->pc = 0;
    c->dstack[1] = 0;   // DL, base address
    c->dstack[2] = 0;   // old base
    c->dstack[3] = 0;   // return address
    c->inscount = 0;
}

void vm_free(vm_context_t *c)
{
    free(c->dstack);
}

uint16_t base(vm_context_t *c, uint16_t l)
{
    uint16_t b1 = c->b;
    while(l > 0)
    {
        b1 = c->dstack[b1];
        l--;
    }
    return b1;
}

int16_t readInt()
{
    int v;
    printf("> ");
    scanf("%d", &v);
    return v;
}

void writeInt(int16_t v)
{
    printf("%d", v);
    fflush(stdout);
}

int16_t readChar()
{
    char v;
    scanf("%c", &v);
    return v;
}

void writeChar(int16_t v)
{
    printf("%c", (char)v);
    fflush(stdout);
}

void vm_push(vm_context_t *c, uint16_t v)
{
    c->t++;
    c->dstack[c->t] = v;
}

bool vm_execute(vm_context_t *c)
{
    instruction_t *ins = (instruction_t *)(c->mem + c->pc*sizeof(instruction_t));
    uint16_t level = 0;
    uint16_t imm16 = ins->opt16;

    c->pc++;
    c->inscount++;
    switch(ins->opcode & 0xF)
    {
    case VM_LIT:    // load literal constant 0,n
        c->t++;
        c->dstack[c->t] = imm16;
        break;
    case VM_OPR:    // arithmetic or logical operation 0,n
        switch(imm16)
        {
        case OPR_RET:   // return from procedure
            c->t  = c->b-1;
            c->pc = c->dstack[c->t+3];
            c->b  = c->dstack[c->t+2];
            break;
        case OPR_NEG:
            c->dstack[c->t] = -c->dstack[c->t];
            break;
        case OPR_ADD:
            c->t--;
            c->dstack[c->t] += c->dstack[c->t+1];
            break;
        case OPR_SUB:
            c->t--;
            c->dstack[c->t] -= c->dstack[c->t+1];
            break;
        case OPR_MUL:
            c->t--;
            c->dstack[c->t] *= c->dstack[c->t+1];
            break;
        case OPR_DIV:
            c->t--;
            c->dstack[c->t] /= c->dstack[c->t+1];
            break;
        case OPR_ODD:
            c->dstack[c->t] = c->dstack[c->t] & 1;
            break;
        case OPR_EQ:
            c->t--;
            c->dstack[c->t] = (c->dstack[c->t] == c->dstack[c->t+1]) ? 1 : 0;
            break;
        case OPR_NEQ:
            c->t--;
            c->dstack[c->t] = (c->dstack[c->t] != c->dstack[c->t+1]) ? 1 : 0;
            break;
        case OPR_LESS:
            c->t--;        
            c->dstack[c->t] = (c->dstack[c->t] < c->dstack[c->t+1]) ? 1 : 0;
            break;            
        case OPR_LEQ:
            c->t--;
            c->dstack[c->t] = (c->dstack[c->t] <= c->dstack[c->t+1]) ? 1 : 0;
            break;            
        case OPR_GREATER:
            c->t--;
            c->dstack[c->t] = (c->dstack[c->t] > c->dstack[c->t+1]) ? 1 : 0;
            break;            
        case OPR_GEQ:
            c->t--;
            c->dstack[c->t] = (c->dstack[c->t] >= c->dstack[c->t+1]) ? 1 : 0;
            break;
        case OPR_OUTCHAR:
            writeChar(c->dstack[c->t]);
            c->t--;
            break;                 
        case OPR_OUTINT:
            writeInt(c->dstack[c->t]);
            c->t--;
            break;                           
        case OPR_INCHAR:
            c->t++;
            c->dstack[c->t] = readChar();
            break;
        case OPR_ININT:
            c->t++;
            c->dstack[c->t] = readInt();
            break;            
        default:
            //error
            break;
        }
        break;
    case VM_LOD:    // load variable l,d
        level = (ins->opcode >> 4); // level
        //printf("Level B = %d\n", base(c,level));
        c->t++;
        c->dstack[c->t] = c->dstack[(uint16_t)(base(c,level) + (int16_t)imm16)];
        //if (level != 0)
        //{
        //    printf("Loading from dstack address %d\n", (uint16_t)(base(c,level) + (int16_t)imm16));
        //}
        break;
    case VM_STO:    // load indexed variable l,d
        level = (ins->opcode >> 4); // level
        //printf("0x%04X\t Level B = %d -> ofs = %d\n", c->pc, base(c,level), imm16);
        c->dstack[(uint16_t)(base(c,level) + (int16_t)imm16)] = c->dstack[c->t];
        c->t--;
        //if (level != 0)
        //{
        //    printf("Storing to dstack address %d\n", (uint16_t)(base(c,level) + (int16_t)imm16));
        //}        
        break;
    case VM_CAL:    // call procedure or function v,a
    {
        level = (ins->opcode >> 4); // level
        uint16_t prevbp = base(c,level);
        c->dstack[c->t+1] = prevbp;
        c->dstack[c->t+2] = c->b;
        c->dstack[c->t+3] = c->pc;
        c->b=c->t+1;
        c->pc=imm16;
    }
        break;
    case VM_INT:    // increment stack pointer 0,n
        c->t += (int16_t)imm16;
        break;
    case VM_JMP:    // unconditional jump 0,a
        c->pc = imm16;
        break;
    case VM_JPC:    // conditional jump 0,a
        if (c->dstack[c->t] == 0)
        {
            c->pc = imm16;
        }
        c->t--;
        break;
    case VM_HALT:
        return false;
    default:
        // error!
        return false;
    }
    return true;
}
