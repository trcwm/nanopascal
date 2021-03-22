/*

    PL/0 lexical analyser
    N.A. Moseley 2021

*/

#pragma once
#include <stdint.h>
#include <stdbool.h>

//#define TOKDUMP

typedef enum
{
    TOK_NONE    = 0,
    TOK_EOF,
    TOK_IDENT,
    TOK_INTEGER,
    TOK_SEMICOL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_STAR,
    TOK_PLUS,
    TOK_MINUS,
    TOK_EQUAL,
    TOK_PERIOD,
    TOK_COMMA,
    TOK_EXCLAMATION,
    TOK_QUESTION,
    TOK_ASSIGN,             // :=    
    TOK_EOL,
    TOK_LEQ,                // <=
    TOK_GEQ,                // >=
    TOK_LESS,               // <
    TOK_GREATER,            // >
    TOK_SLASH,              // /
    TOK_HASH,               // #
    TOK_PROGRAM = 100,
    TOK_BEGIN,
    TOK_END,
    TOK_VAR,
    TOK_WHILE,
    TOK_DO,
    TOK_PROCEDURE,
    TOK_CALL,
    TOK_CONST,
    TOK_IF,
    TOK_THEN,
    TOK_ODD,
    TOK_ELSE,
    TOK_SHR,
    TOK_SHL,
    TOK_SAR,
    TOK_FOR,
    TOK_TO,
    TOK_DOWNTO 
} token_t;

typedef enum
{
    LS_IDLE = 0,
    LS_INTEGER,
    LS_IDENT,
    LS_LINECOMMENT
} lexstate_t;

/** lexical analyser context data */
typedef struct
{
    char    *src;       ///< pointer to the source code
    char    *tokstart;  ///< pointer to start of current token string
    int16_t toklen;     ///< length of current token
    token_t token;      ///< current token type
    lexstate_t state;   ///< analyser state
    int16_t linenum;    ///< current line number
    uint16_t number;    ///< value of integer literal
} lexer_context_t;

/** initialise the lexical analyser */
void lexer_init(lexer_context_t *context, char *source);

/** generate next token */
bool lexer_next(lexer_context_t *context);