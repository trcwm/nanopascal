/*

    Lexer for p-code assembler
    N.A. Moseley (c) 2021

*/

#pragma once
#include <stdint.h>

typedef enum 
{
    TOK_UNDEFINED = 0,
    TOK_INTEGER   = 1,
    TOK_IDENT     = 2,
    TOK_EOL       = 3,
    TOK_LABEL     = 4,

    /* keywords */
    TOK_LIT = 100,
    TOK_OPR = 101,
    TOK_LOD = 102,
    TOK_STO = 103,
    TOK_CAL = 104,
    TOK_INT = 105,
    TOK_JMP = 106,
    TOK_JPC = 107,
    TOK_HALT = 108,
    TOK_RET = 109,
    TOK_NEG = 110,
    TOK_ADD = 111,
    TOK_SUB = 112,
    TOK_MUL = 113,
    TOK_DIV = 114,
    TOK_ODD = 115,
    TOK_LES = 116,
    TOK_LEQ = 117,
    TOK_GRE = 118,
    TOK_GEQ = 119,
    TOK_NEQ = 120,
    TOK_EQU = 121,
    TOK_READ = 122,
    TOK_WRITE = 123,
    TOK_EOF = 255
} token_t;

typedef enum
{
    LS_IDLE    = 0,
    LS_INTEGER, 
    LS_IDENT,
    LS_LABEL,
    LS_COMMENT,
    LS_HEX
} lexstate_t;

typedef struct 
{
    const char *src;
    int     srcidx; 
    
    lexstate_t lstate;

    const char *tokstr;
    int         toklen;
    uint16_t    lit;        ///< literal integer or hex number

    token_t curtok;
    int     linenum;
} lex_context_t;

void lex_init(lex_context_t *context, const char *sourcePtr);
void lex_free(lex_context_t *context);
void lex_next(lex_context_t *context);
