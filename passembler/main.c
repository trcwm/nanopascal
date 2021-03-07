/*

    p-code assembler
    Niels A. Moseley (c) 2021

*/

#include <stdio.h>
#include <stdlib.h>
#include "lex.h"
#include "parse.h"

int main(int argc, char *argv[])
{
    printf("p-code assembler v0.1\n\n");

    if (argc < 2)
    {
        printf("Usage: %s <infile>\n", argv[0]);
        return -1;
    }

    FILE *fin = fopen(argv[1],"rb");
    if (fin == 0)
    {
        printf("Could not read file %s\n", argv[1]);
        return -1;
    }   

    fseek(fin,0,SEEK_END);
    size_t bytes = ftell(fin);
    rewind(fin);

    printf("Loading %lu bytes\n", bytes);

    char *src = malloc(bytes);
    if (fread(src, 1, bytes, fin) != bytes)
    {
        printf("Could not read file %s\n", argv[1]);
        return -1;
    }

#if 0
    lex_context_t lex;
    lex_init(&lex, src);

    while(lex.curtok != TOK_EOF)
    {
        lex_next(&lex);
        switch(lex.curtok)
        {
        case TOK_EOL:
            printf("TOK: EOL\n");
            break;
        case TOK_EOF:
            printf("TOK: EOF\n");
            break;
        default:
            printf("TOK: %s (%d)\n", lex.tokstr, lex.curtok);
            break; 
        }
    }

    free(src);
    lex_free(&lex);
#else
    parse(src);
#endif
}
