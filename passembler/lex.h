#pragma once

/*

    Lexer

*/

typedef enum 
{
    TOK_UNDEFINED = 0,
    TOK_INTEGER   = 1,
    TOK_IDENT     = 2,
    TOK_EOL       = 3,
    
    /* keywords */
    TOK_LIT = 100,
    TOK_OPR = 101,
    TOK_LOD = 102,
    TOK_STO = 103,
    TOK_CAL = 104,
    TOK_INT = 105,
    TOK_JMP = 106,
    TOK_JPC = 107,
    TOK_EOF = 255
} token_t;

typedef enum
{
    LS_IDLE    = 0,
    LS_INTEGER = 1,
    LS_IDENT   = 2
} lexstate_t;

typedef struct 
{
    const char *src;
    int     srcidx; 
    
    lexstate_t lstate;

    char    tokstr[100];
    int     tokidx;

    token_t curtok;
} lex_context_t;

void lex_init(lex_context_t *context, const char *sourcePtr);
void lex_free(lex_context_t *context);
void lex_next(lex_context_t *context);
