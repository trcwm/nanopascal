#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../passembler/keywords.h"
#include "../virtualmachine/vm.h"

void printalu(uint16_t imm16)
{
    switch(imm16)
    {
    case OPR_RET:
        printf("RET\n");
        break;
    case OPR_NEG:
        printf("NEG\n");
        break;
    case OPR_ADD:    
        printf("ADD\n");
        break;
    case OPR_SUB:    
        printf("SUB\n");
        break;    
    case OPR_MUL:    
        printf("SUB\n");
        break;            
    case OPR_DIV:    
        printf("SUB\n");
        break;            
    case OPR_ODD:    
        printf("ODD\n");
        break;
    case OPR_EQ:
        printf("EQU\n");
        break;                   
    case OPR_NEQ:
        printf("NEQ\n");
        break;             
    case OPR_LESS:
        printf("LES\n");
        break;     
    case OPR_LEQ:
        printf("LEQ\n");
        break;     
    case OPR_GREATER:
        printf("GRE\n");
        break;     
    case OPR_GEQ:
        printf("GEQ\n");
        break;                                                
    default:
        printf("???\n");
        break;
    }
}

void printins(const instruction_t *ins)
{
    uint8_t     opcode = ins->opcode;
    uint16_t    imm16  = ins->opt16;
    switch (opcode & 0xF)
    {
    case VM_LIT:
        printf("LIT %d\n", imm16);
        break;
    case VM_OPR:
        printalu(imm16);
        break;
    case VM_LOD:
        printf("LOD lvl:%d  ofs:%d\n", opcode >> 4 ,imm16);
        break;
    case VM_STO:
        printf("STO lvl:%d  ofs:%d\n", opcode >> 4 ,imm16);
        break;
    case VM_CAL:
        printf("CAL lvl:%d  0x%04X\n", opcode >> 4, imm16);
        break;
    case VM_INT:
        printf("INT %d\n", imm16);
        break;
    case VM_JMP:
        printf("JMP 0x%04X\n", imm16);
        break;
    case VM_JPC:
        printf("JPC 0x%04X\n", imm16);
        break;
    case VM_HALT:
        printf("HALT\n");
        break; 
    case VM_ININT:
        printf("ININT\n");
        break;
    case VM_OUTINT:
        printf("OUTINT\n");
        break;        
    case VM_OUTCHAR:
        printf("OUTCHAR\n");
        break;                        
    default:
        printf("???\n");
        break;
    }
}

int main(int argc, char *argv[])
{
    printf("; p-code disassembler v0.1\n");

    if (argc < 2)
    {
        printf("Usage: %s <infile>\n", argv[0]);
        return -1;
    }

    FILE *fin = fopen(argv[1],"rb");
    if (fin == 0)
    {
        printf("Could not read file %s\n", argv[1]);
        return -1;
    }   

    fseek(fin,0,SEEK_END);
    size_t bytes = ftell(fin);
    rewind(fin);

    printf("; Loading %lu bytes\n", bytes);

    char *src = malloc(bytes);
    if (fread(src, 1, bytes, fin) != bytes)
    {
        printf("Could not read file %s\n", argv[1]);
        return -1;
    }

    for(uint16_t ofs=0; ofs<bytes; ofs+=3)
    {
        printf("0x%04X:\t", ofs/3);
        printins((instruction_t*) &src[ofs]);
    }

    free(src);
}
