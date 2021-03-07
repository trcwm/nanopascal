#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lex.h"

void next(lex_context_t *lex)
{
    lex_next(lex);
}

bool parseLine(lex_context_t *lex)
{
    // skip empty lines
    if (lex->curtok == TOK_EOL)
    {
        next(lex);
        return true;
    }

    if (lex->curtok < 100)
    {
        printf("Expected opcode\n");
        return false;
    }

    uint8_t opcode = lex->curtok - 100;
    next(lex);

    if (lex->curtok != TOK_INTEGER)
    {
        printf("Expected integer after opcode\n");
        return false;
    }

    opcode |= (atoi(lex->tokstr) << 4);
    printf("0x%02X,", opcode);

    next(lex);

    if (lex->curtok != TOK_INTEGER)
    {
        printf("Expected second integer after opcode\n");
        return false;
    }

    uint16_t word = atoi(lex->tokstr);

    printf("0x%02X,", word & 0x0FF);
    printf("0x%02X,\n", (word >> 8) & 0x0FF);

    while((lex->curtok != TOK_EOL) && (lex->curtok != TOK_EOF))
    {
        next(lex);
    }
    return true;
}

void parse(const char *src)
{
    lex_context_t lex;
    lex_init(&lex, src);

    while(lex.curtok != TOK_EOF)
    {
        parseLine(&lex);
    }
}
