/*

    Symbol table

*/

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum
{
    TYPE_NONE,
    TYPE_INT,
    TYPE_CONST,
    TYPE_PROCEDURE
} symtype_t;

/** symbol table entry */
typedef struct 
{
    symtype_t   type;       // symbol type
    uint8_t     level;      // nesting level
    uint16_t    offset;     // offset into local stack    
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
bool sym_add(symtbl_t *tbl, const symtype_t tp, const char *name, uint16_t namelen);
bool sym_addproc(symtbl_t *tbl, const symtype_t tp, const char *name, uint16_t namelen, uint16_t address);
bool sym_enter(symtbl_t *tbl);
bool sym_leave(symtbl_t *tbl);

sym_t* sym_lookup(symtbl_t *tbl, const char *name, uint16_t namelen);

void sym_dump(symtbl_t *tbl);