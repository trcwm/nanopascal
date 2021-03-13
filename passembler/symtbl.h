/*

    Symbol table for p-code assembler
    Niels A. Moseley (c) 2021

*/

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/** symbol table entry */
typedef struct 
{
    uint16_t    address;    // address of label
    char        *name;      // name of symbol
    uint16_t    namelen;
} sym_t;

#define MAX_SYMS 1000

/** symbol table for labels */
typedef struct
{
    uint16_t    Nsymbols;   ///< number of symbols in the table
    sym_t syms[MAX_SYMS];   ///< storage for symbols
} symtbl_t;

bool sym_init(symtbl_t *tbl);
bool sym_add(symtbl_t *tbl, const char *name, uint16_t namelen, uint16_t address);
sym_t* sym_lookup(symtbl_t *tbl, const char *name, uint16_t namelen);
void sym_dump(symtbl_t *tbl);
