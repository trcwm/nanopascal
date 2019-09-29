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
    vm_context_t *c = malloc(sizeof(vm_context_t));

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

    return 0;
}
