/*

  A virtual machine reference implementation
  for the Nano Pascal compiler

  Copyright Niels A. Moseley 2017 - 2021
  Moseley Instruments
  http://www.moseleyinstruments.com


*/

#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>

#include "vm.h"

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

int main(int argc, char *argv[])
{
#if 0    
    // program to multiply x and y together and
    // write result in z.
    uint8_t mem[] =
    {
        0x04,0x02,0x00, // call procedure at address 0x02
        0x06,0x02,0x00, // nop
        0x05,0x05,0x00,
        0x12,0x03,0x00,
        0x03,0x03,0x00,
        0x12,0x04,0x00,
        0x03,0x04,0x00,
        0x00,0x00,0x00,
        0x13,0x05,0x00,
        0x02,0x04,0x00,
        0x00,0x00,0x00, // 10
        0x01,0x0C,0x00,
        0x07,0x1D,0x00,
        0x02,0x04,0x00,
        0x01,0x06,0x00,
        0x07,0x14,0x00,
        0x12,0x05,0x00,
        0x02,0x03,0x00,
        0x01,0x02,0x00,
        0x13,0x05,0x00,
        0x00,0x02,0x00, //20
        0x02,0x03,0x00,
        0x01,0x04,0x00,
        0x03,0x03,0x00,
        0x02,0x04,0x00,
        0x00,0x02,0x00,
        0x01,0x05,0x00,
        0x03,0x04,0x00,
        0x06,0x09,0x00,
        0x08,0x00,0x00  // halt
        };
#endif

    printf("P-code virtual machine 0.1\n");
    printf("Instruction size is %lu bytes\n\n", sizeof(instruction_t));

    uint8_t *mem = NULL;

    if (argc < 2)
    {
        printf("Usage: %s <code.bin>\n", argv[0]);  
        return -1;      
    }
    else
    {
        FILE *fin = fopen(argv[1],"rb");
        if (fin == NULL)
        {
            printf("Cannot open file %s\n", argv[1]);
            return -1;
        }

        fseek(fin, 0, SEEK_END);
        size_t bytes = ftell(fin);
        rewind(fin);

        mem = malloc(bytes);
        if (fread(mem, 1, bytes, fin) != bytes)
        {
            printf("Cannot read file %s\n", argv[1]);
            return -1;
        }

        fclose(fin);
    }

    vm_context_t vm;
    vm_init(&vm, mem);

#if 1
    vm_push(&vm, 0);
    vm_push(&vm, 0);
    vm_push(&vm, 0);
#endif    

    while(1)
    {
#if 0     
        printf("PC: %04X (%02X)\tT: %d\tB: %d\t dstack: %05d %05d\n", vm.pc, vm.mem[3*vm.pc], vm.t, vm.b, vm.dstack[vm.t], vm.dstack[vm.t-1]);
        uint8_t     opcode = vm.mem[vm.pc * sizeof(instruction_t)];
        uint16_t    imm16  = vm.mem[vm.pc * sizeof(instruction_t)+1] | (vm.mem[vm.pc * sizeof(instruction_t)+2]<<8);
        switch (opcode & 0xF)
        {
        case VM_LIT:
            printf("LIT %d\n", imm16);
            break;
        case VM_OPR:
            printalu(imm16);
            //printf("OPR %d\n", imm16);
            break;
        case VM_LOD:
            printf("LOD %d %d\n", opcode >> 4 ,imm16);
            break;
        case VM_STO:
            printf("STO %d %d\n", opcode >> 4 ,imm16);
            break;
        case VM_CAL:
            printf("CAL 0x%04X\n", imm16);
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
#endif            
        if (vm_execute(&vm) == false)
            break;
    }

    printf("Executed %d instructions\n", vm.inscount);
    vm_free(&vm);
    free(mem);
    return 0;
}
