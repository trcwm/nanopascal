#include <stdbool.h>
#include <string.h>
#include "lexer.h"

// define PL/0 keywords
// the order must be the same
// as the TOK_ definitions starting at value 100
#define NKEYWORDS 12
const char* keywords[NKEYWORDS] =
{
    "PROGRAM",
    "BEGIN",
    "END",
    "VAR",
    "WHILE",
    "DO",
    "PROCEDURE",
    "CALL"
    "CONST",
    "IF",
    "THEN",
    "ODD"
};

void lexer_init(lexer_context_t *context, char *source)
{
    context->src      = source;
    context->tokstart = source;
    context->toklen   = 0;
    context->token    = TOK_NONE;
    context->state    = LS_IDLE;
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
        if (strncmp(context->tokstart, keywords[kwindex], context->toklen) == 0)
        {
            context->token = 100 + kwindex;
            return;
        }
    }
}

// generate the next token
bool lexer_next(lexer_context_t *context)
{    
    // advance past the previously emitted token
    context->tokstart += context->toklen;
    context->toklen    = 0;

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
                context->state = LS_INTEGER;
                lexer_accept(context);
            }
            else if (c == 10)
            {
                lexer_accept(context);
                lexer_emit(context, TOK_EOL);
                return true;
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
                lexer_accept(context);
            }
            else
            {
                lexer_emit(context, TOK_INTEGER);
                context->state = LS_IDLE;
                return true;
            }
            break;
        default:
            return false; // incorrect lexer state       
        }
    }
}
