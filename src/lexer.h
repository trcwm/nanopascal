#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    TOK_NONE    = 0,
    TOK_EOF,
    TOK_IDENT,
    TOK_INTEGER,
    TOK_EOL   
} token_t;

typedef enum
{
    LS_IDLE = 0,
    LS_INTEGER,
    LS_IDENT
} lexstate_t;

/** lexical analyser context data */
typedef struct
{
    char    *src;       ///< pointer to the source code
    char    *tokstart;  ///< pointer to start of current token string
    int16_t toklen;     ///< length of current token    
    token_t token;      ///< current token type
    lexstate_t state;   ///< analyser state
} lexer_context_t;

/** initialise the lexical analyser */
void lexer_init(lexer_context_t *context, char *source);

/** generate next token */
bool lexer_next(lexer_context_t *context);