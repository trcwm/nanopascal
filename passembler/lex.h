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
    TOK_OPR,
    TOK_LOD,
    TOK_STO,
    TOK_CAL,
    TOK_INT,
    TOK_JMP,
    TOK_JPC,
    TOK_OUTCHAR,
    TOK_OUTINT,
    TOK_ININT,    
    TOK_HALT,
    TOK_RET,
    TOK_NEG,
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_ODD,
    TOK_LES,
    TOK_LEQ,
    TOK_GRE,
    TOK_GEQ,
    TOK_NEQ,
    TOK_EQU,
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
