/*

    PL/0 lexical analyser
    N.A. Moseley 2021

*/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"

// define PL/0 keywords
// the order must be the same
// as the TOK_ definitions starting at value 100
#define NKEYWORDS 16
const char* keywords[NKEYWORDS] =
{
    "PROGRAM",
    "BEGIN",
    "END",
    "VAR",
    "WHILE",
    "DO",
    "PROCEDURE",
    "CALL",
    "CONST",
    "IF",
    "THEN",
    "ODD",
    "ELSE",
    "SHR",
    "SHL",
    "SAR",
};

void lexer_init(lexer_context_t *context, char *source)
{
    context->src      = source;
    context->tokstart = source;
    context->toklen   = 0;
    context->token    = TOK_NONE;
    context->state    = LS_IDLE;
    context->linenum  = 1;
}

bool isWhitespace(const char c)
{
    return (c==' ') || (c=='\t');
}

bool isAlpha(const char c)
{
    return ((c>='A') && (c<='Z')) || ((c>='a') && (c<='z'));
}

bool isNumeric(const char c)
{
    return (c>='0') && (c<='9');
}

bool isAlphaNum(const char c)
{
    return isAlpha(c) || isNumeric(c);
}

char lex_toupper(const char c)
{
    if ((c >= 'a') && (c <='z'))
    {
        return c - 'a' + 'A';
    }
}

// add the current character to the token string
void lexer_accept(lexer_context_t *context)
{
    context->toklen++;
}

// skip the current character and reset the token string
void lexer_skip(lexer_context_t *context)
{
    context->tokstart++;
    context->toklen = 0;
}

// set the token ID and return the lexer to the idle state
void lexer_emit(lexer_context_t *context, token_t tok)
{
    context->token = tok;
    context->state = LS_IDLE;
    #ifdef TOKDUMP
        printf("lex:    ");
        for(int i=0; i<context->toklen; i++)
        {
            putchar(context->tokstart[i]);
        }
        printf(" (%d)\n", context->token);
    #endif
}

// look-ahead the next character
char lexer_peekNextChar(lexer_context_t *context)
{
    return context->tokstart[context->toklen+1];
}

// check if the current token string is a keyword
// and set the token accordingly
void lexer_checkKeyword(lexer_context_t *context)
{
    for(int kwindex=0; kwindex<NKEYWORDS; kwindex++)
    {
        const char *kw = keywords[kwindex];
        
        if (kw[context->toklen] != 0)
        {
            // keyword and token are not the same length
            // so skip this keyword
            continue;
        }

        bool fail = false;        
        for(int cindex=0; cindex < context->toklen; cindex++)
        {
            if (kw[cindex] != lex_toupper(context->tokstart[cindex]))
            {
                fail = true;
                break;
            }
        }
        
        if (fail)
            continue;

        context->token = 100 + kwindex;
        return;
    }
}

// generate the next token
bool lexer_next(lexer_context_t *context)
{    
    // advance past the previously emitted token
    context->tokstart += context->toklen;
    context->toklen    = 0;
    context->number    = 0;
    while(1)
    {
        char c = context->tokstart[context->toklen];

        if (c == 0)
        {
            lexer_emit(context, TOK_EOF);
            return true;
        }

        switch(context->state)
        {
        case LS_IDLE:
            context->toklen = 0;

            // skip whitespace
            if (isWhitespace(c))
            {
                lexer_skip(context);
            }
            else if (isAlpha(c))
            {
                context->state = LS_IDENT;
                lexer_accept(context);
            }
            else if (isNumeric(c))
            {
                context->number = c-'0';
                context->state = LS_INTEGER;
                lexer_accept(context);
            }
            else if (c == 10)
            {
                context->linenum++;
#ifdef SUPPORT_EOL
                lexer_accept(context);
                lexer_emit(context, TOK_EOL);
                return true;
#else
                lexer_skip(context);
#endif
            }
            else
            {
                // some kind of operator?
                switch(c)
                {
                case ';':
                    lexer_accept(context);
                    lexer_emit(context, TOK_SEMICOL);
                    return true;
                case '-':
                    lexer_accept(context);
                    lexer_emit(context, TOK_MINUS);
                    return true;
                case '+':
                    lexer_accept(context);
                    lexer_emit(context, TOK_PLUS);
                    return true;
                case '*':
                    lexer_accept(context);
                    lexer_emit(context, TOK_STAR);
                    return true;
                case '=':
                    lexer_accept(context);
                    lexer_emit(context, TOK_EQUAL);
                    return true;                    
                case '(':
                    lexer_accept(context);
                    lexer_emit(context, TOK_LPAREN);
                    return true;
                case ')':
                    lexer_accept(context);
                    lexer_emit(context, TOK_RPAREN);                    
                    return true; 
                case '.':
                    lexer_accept(context);
                    lexer_emit(context, TOK_PERIOD);
                    return true;                      
                case ',':
                    lexer_accept(context);
                    lexer_emit(context, TOK_COMMA);
                    return true;   
                case '/':
                    // check if this is a line comment or a regular
                    // single /
                    if (lexer_peekNextChar(context) == '/')
                    {
                        lexer_skip(context);
                        lexer_skip(context);
                        context->state = LS_LINECOMMENT;
                    }
                    else
                    {
                        lexer_accept(context);
                        lexer_emit(context, TOK_SLASH);
                        return true;
                    }
                    break;
                case '#':
                    lexer_accept(context);
                    lexer_emit(context, TOK_HASH);
                    return true;                    
                case '!':
                    lexer_accept(context);
                    lexer_emit(context, TOK_EXCLAMATION);
                    return true;
                case '?':
                    lexer_accept(context);
                    lexer_emit(context, TOK_QUESTION);
                    return true;                    
                case ':':
                    // check if this is an assign operator
                    if (lexer_peekNextChar(context) == '=')
                    {
                        lexer_accept(context);
                        lexer_accept(context);
                        lexer_emit(context, TOK_ASSIGN);
                        return true;
                    }
                    else
                    {
                        // FIXME: is this an error?
                        // can the ':' appear by itself in PL/0?
                        return false;
                    }
                    break;
                case '<':
                    // check if this < or <=
                    if (lexer_peekNextChar(context) == '=')
                    {
                        lexer_accept(context);
                        lexer_accept(context);
                        lexer_emit(context, TOK_LEQ);
                        return true;
                    }
                    else
                    {
                        lexer_accept(context);
                        lexer_emit(context, TOK_LESS);
                        return true;
                    }
                    break;
                case '>':
                    // check if this > or >=
                    if (lexer_peekNextChar(context) == '=')
                    {
                        lexer_accept(context);
                        lexer_accept(context);
                        lexer_emit(context, TOK_GEQ);
                        return true;
                    }
                    else
                    {
                        lexer_accept(context);
                        lexer_emit(context, TOK_GREATER);
                        return true;
                    }
                    break;
                default:
                    // skip everything else
                    lexer_skip(context);
                }
            }
            break;
        case LS_IDENT:
            if (isAlphaNum(c))
            {
                lexer_accept(context);
            }
            else
            {
                lexer_emit(context, TOK_IDENT);
                lexer_checkKeyword(context);
                context->state = LS_IDLE;
                return true;
            }
            break;
        case LS_INTEGER:
            if (isNumeric(c))
            {
                uint16_t orig = context->number;
                context->number <<= 2;
                context->number += orig;
                context->number <<= 1;
                context->number += c - '0';
                lexer_accept(context);
            }
            else
            {
                lexer_emit(context, TOK_INTEGER);
                context->state = LS_IDLE;
                return true;
            }
            break;
        case LS_LINECOMMENT:
            if (c == 10)
            {
                context->state = LS_IDLE;
            }
            else
            {
                lexer_skip(context);
            }
            break;
        default:
            return false; // incorrect lexer state       
        }
    }
}
