/*

    Pascal to p-code compiler
    N.A. Moseley 2021

*/

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    printf("PL/0 to p-code compiler\n\n");

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

    lexer_context_t lex;
    lexer_init(&lex, src);

    while(lex.token != TOK_EOF)
    {
        lexer_next(&lex);
        switch(lex.token)
        {
        case TOK_EOL:
            printf("TOK: EOL\n");
            break;
        case TOK_EOF:
            printf("TOK: EOF\n");
            break;
        default:
            printf("TOK: ");
            for(int i=0; i<lex.toklen; i++)
            {
                putchar(lex.tokstart[i]);
            }
            printf(" (%d)\n", lex.token);
            break; 
        }
    }

    free(src);
    //lex_free(&lex);
}
