/*

    Lexer for p-code assembler
    N.A. Moseley (c) 2021

*/

#pragma once
#include <stdint.h>
#include "keywords.h"

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
