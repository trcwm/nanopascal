/*

    call/jump fix-up table for p-code assembler
    Niels A. Moseley (c) 2021

*/

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/** fixup table entry */
typedef struct 
{
    uint16_t    address;    // address of instruction to fix up
    char        *name;      // name of label
    uint16_t    namelen;
} fixup_t;

#define MAX_FIXES 1000

/** symbol table for labels */
typedef struct
{
    uint16_t    Nentries;           ///< number of fixups in the table
    fixup_t     fixups[MAX_FIXES];  ///< storage for fixups
} fixtbl_t;

bool fix_init(fixtbl_t *tbl);
bool fix_add(fixtbl_t *tbl, const char *name, uint16_t namelen, uint16_t address);
void fix_dump(fixtbl_t *tbl);
