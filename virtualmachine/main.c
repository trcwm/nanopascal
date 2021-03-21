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

int main(int argc, char *argv[])
{
    printf("P-code virtual machine 0.1\n");
    printf("Instruction size is %lu bytes\n\n", sizeof(instruction_t));

    uint8_t *mem = NULL;

    size_t bytes = 0;
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
        bytes = ftell(fin);
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
    vm_init(&vm, mem, bytes);

    while(1)
    {          
        if (vm_execute(&vm) == false)
            break;
    }

    printf("Executed %lu instructions\n", vm.inscount);
    vm_free(&vm);
    free(mem);
    return 0;
}
