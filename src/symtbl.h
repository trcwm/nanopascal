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
    vartype_t   subtype;    // sub type (arrays only)
    uint8_t     level;      // nesting level
    uint16_t    offset;     // offset into local stack, or label id
    uint16_t    size;       // size of variable in 16-bit words
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

/** add a symbol to the symbol table in name only.
 *  no space will be allocated on the stack until
 *  sym_allocate is called 
 * 
 *  the type will be set to TYPE_NONE.
 * 
 *  fill in the rest of the symbol information using
 *  sym_settype and symbol offset is set to zero.
 * */
bool sym_add(symtbl_t *tbl, const char *name, uint16_t namelen);

/** enter a new procedure/block level - increments level */
bool sym_enter(symtbl_t *tbl);

/** leave a new procedure/block level - decrements level */
bool sym_leave(symtbl_t *tbl);

/** set type information and update the offsets */
bool sym_update(symtbl_t *tbl, const uint16_t id, const vartype_t tp, const vartype_t subtp, 
    const uint16_t size);

/** set type information of a constant */
bool sym_set_const(symtbl_t *tbl, const uint16_t id, const uint16_t value);

/** set type information of a procedure */
bool sym_set_procedure(symtbl_t *tbl, const uint16_t id, const uint16_t label);

/** calculate the number of local variables */
uint16_t sym_numvariables(symtbl_t *tbl);

/** calculate the number of 16-bit cells for local variables */
uint16_t sym_get_local_space(symtbl_t *tbl);

sym_t* sym_lookup(symtbl_t *tbl, const char *name, uint16_t namelen);

void sym_dump(symtbl_t *tbl);