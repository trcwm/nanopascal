/*

  A virtual machine reference implementation
  for the Micro Pascal compiler

  Copyright Niels A. Moseley 2017 - 2019
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>

#include "vm.h"

int main(int argc, char *argv[])
{
    uint8_t mem[] =
        {
            0x06, 0x02, 0x00,
            0x06, 0x02, 0x00,
            0x05, 0x05, 0x00,
            0x12, 0x03, 0x00,
            0x03, 0x03, 0x00,
            0x12, 0x04, 0x00,
            0x03, 0x04, 0x00,
            0x00, 0x00, 0x00,
            0x13, 0x05, 0x00,
            0x02, 0x04, 0x00,
            0x00, 0x00, 0x00,
            0x01, 0x0C, 0x00,
            0x07, 0x1D, 0x00,
            0x02, 0x04, 0x00,
            0x01, 0x06, 0x00,
            0x07, 0x14, 0x00,
            0x12, 0x05, 0x00,
            0x02, 0x03, 0x00,
            0x01, 0x02, 0x00,
            0x13, 0x05, 0x00,
            0x00, 0x02, 0x00,
            0x02, 0x03, 0x00,
            0x01, 0x04, 0x00,
            0x03, 0x03, 0x00,
            0x02, 0x04, 0x00,
            0x00, 0x02, 0x00,
            0x01, 0x05, 0x00,
            0x03, 0x04, 0x00,
            0x06, 0x09, 0x00,
            0x01, 0x00, 0x00};

    printf("P-code virtual machine 0.1\n");
    printf("Instruction size is %lu bytes\n\n", sizeof(instruction_t));

    vm_context_t vm;
    vm_init(&vm, mem);

    //vm_push(&vm, 0);
    //vm_push(&vm, 4);

    for (uint32_t i = 0; i < 32; i++)
    {
        printf("PC: %d\t", vm.pc);
        uint8_t opcode = vm.mem[vm.pc * sizeof(instruction_t)];
        switch (opcode & 0xF)
        {
        case VM_LIT:
            printf("LIT\n");
            break;
        case VM_OPR:
            printf("OPR\n");
            break;
        case VM_LOD:
            printf("LOD\n");
            break;
        case VM_STO:
            printf("STO\n");
            break;
        case VM_CAL:
            printf("CAL\n");
            break;
        case VM_INT:
            printf("INT\n");
            break;
        case VM_JMP:
            printf("JMP\n");
            break;
        case VM_JPC:
            printf("JPC\n");
            break;
        default:
            printf("???\n");
            break;
        }
        vm_execute(&vm);
    }

    vm_free(&vm);

#if 0
    if (argc==2)
    {
        vm_loadfile(c, argv[1]);
    }
    else
    {
        printf("Need a file name as argument!\n");
        return 1;
    }

    while(vm_execute(c))
    {
    }
    free(c);
#endif
    return 0;
}
