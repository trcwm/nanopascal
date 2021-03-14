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
    printf("; p-code assembler v0.1\n\n");

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

    printf("; Loading %lu bytes\n", bytes);

    char *src = malloc(bytes);
    if (fread(src, 1, bytes, fin) != bytes)
    {
        printf("Could not read file %s\n", argv[1]);
        return -1;
    }

    parse(src);
}
