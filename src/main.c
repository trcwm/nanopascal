/*

    Pascal to p-code compiler
    N.A. Moseley 2021

*/

#include <stdio.h>
#include <stdlib.h>
//#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    printf("; PL/0 to p-code compiler\n");
    printf("; Compiled on " __DATE__ "\n\n");

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

    printf("; file = %s\n", argv[1]);
    printf("; Loading %lu bytes\n\n", bytes);

    char *src = malloc(bytes);
    if (fread(src, 1, bytes, fin) != bytes)
    {
        printf("Could not read file %s\n", argv[1]);
        return -1;
    }

    if (!parse(src))
    {
        fprintf(stderr, "Parse failed!\n");
        return -1;
    }
    else
    {
        fprintf(stderr, "Parse ok!\n");
    }

    free(src);
    return 0;
}
