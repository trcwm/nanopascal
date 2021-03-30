/*

    Symbol table

*/

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ptypes.h"

/** symbol table entry */
typedef struct 
{
    vartype_t   type;       // symbol type
    uint8_t     level;      // nesting level
    uint16_t    offset;     // offset into local stack, or label id   
    char        *name;      // name of symbol    
    uint16_t    namelen;    // length of name
} sym_t;

#define MAX_SYMS 200

/** symbol table */
typedef struct
{
    uint16_t    Nsymbols;   ///< number of symbols in the table
    uint8_t     level;      ///< current max. nexting level of the table
    uint16_t    offset;     ///< current offset into local stack
    sym_t syms[MAX_SYMS];   ///< storage for symbols
} symtbl_t;

bool sym_init(symtbl_t *tbl);
bool sym_add(symtbl_t *tbl, const vartype_t tp, const char *name, uint16_t namelen);
bool sym_addproc(symtbl_t *tbl, const vartype_t tp, const char *name, uint16_t namelen, uint16_t address);

/** enter a new procedure/block level - increments level */
bool sym_enter(symtbl_t *tbl);

/** leave a new procedure/block level - decrements level */
bool sym_leave(symtbl_t *tbl);

/** set type of last entered symbols that have type set to TYPE_NONE */
void sym_settype(symtbl_t *tbl, const vartype_t tp);

/** calculate the number of local variables */
uint16_t sym_numvariables(symtbl_t *tbl);

sym_t* sym_lookup(symtbl_t *tbl, const char *name, uint16_t namelen);

void sym_dump(symtbl_t *tbl);