/*

  Micro Pascal
  Niels A. Moseley
  Moseley Instruments
  www.moseleyinstruments.com

  Licence: GPL v2.0

*/

//#include "functiondefs.h"
#include <iostream>
#include <stdio.h>
#include "reader.h"
#include "tokenizer.h"
#include "parser.h"
#include "pcodegenerator.h"

#define __VERSION__ "0.1"

int main(int argc, char *argv[])
{
    bool dumpAST = true;
    bool dumpTokens = false;

    printf("Micro Pascal version " __VERSION__"\n");
    printf("By Niels Moseley\n");
    printf("www.moseleyinstruments.com\n\n");

    if (argc < 2)
    {
        printf("Usage: %s <infile.pas>", argv[0]);
        return 1;
    }

    printf("Tokenizing..\n");
    Reader *reader = Reader::create(argv[1]);
    if (reader == NULL)
    {
        return 1;
    }

    Tokenizer tokenizer;

    std::vector<token_t> tokens;
    tokenizer.process(reader, tokens);
    delete reader;

    if (dumpTokens)
    {
        printf("Dumping tokens:\n");
        for(uint32_t i=0; i<tokens.size(); i++)
        {
            printf("%03d ID: %d -> %s\n", i, tokens[i].tokID, tokens[i].txt.c_str());
        }
    }

    printf("Parsing..\n");
    ParseContext context;
    Parser parser;

    if (parser.process(tokens, context))
    {
        // ok!
        printf("Parsing successful\n");

        if (dumpAST)
        {
            AST::dumpASTree(context.m_astHead);
        }

        AST::dumpASTreeToDot(context.m_astHead, "program.dot");

        std::vector<uint8_t> code;
        PCodeGenerator generator(false, true);
        generator.process(context.m_astHead, code);

        printf("Disassembled VM code:\n");
        VM::disassemble(code);

        if (code.size() == 0)
        {
            printf("Error: code generator didn't emit any code!\n");
        }
        else
        {
            FILE *fout=fopen("tests\\bytecode.dat", "wb");
            fwrite(&code[0], 1, code.size(), fout);
            fclose(fout);
        }
    }
    else
    {
        printf("Parsing failed\n");
    }

    return 0;
}
