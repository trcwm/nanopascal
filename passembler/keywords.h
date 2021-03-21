#pragma once

#define NKEYWORDS 25
extern const char* keywords[NKEYWORDS];

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
    // OPR functions:
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


