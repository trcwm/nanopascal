/*

    Parser for p-code assembler
    N.A. Moseley (c) 2021

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse.h"

typedef struct 
{
    lex_context_t lex;
    uint16_t emitaddress;   ///< address of next emitted instruction
} parse_context_t;

void emit8(const unsigned char byte)
{
    static int ecount = 0;
    ecount++;
    if (ecount == 3)
    {
        printf("%02X\n", byte);
        ecount = 0;
    }
    else
    {
        printf("%02X ", byte);
    }    
}

void next(parse_context_t *context)
{
    lex_next(&context->lex);
}

token_t token(parse_context_t *context)
{
    return context->lex.curtok;
}

void parse_error(parse_context_t *context, const char *str)
{
    printf("\nLine %d: %s", context->lex.linenum, str);
    printf("  tokstr: '");
    for(uint16_t i=0; i<context->lex.toklen; i++)
        putchar(context->lex.tokstr[i]);
    printf("'\n");
}

// instruction (level|aluop) (offset | addr)
bool parse_instruction(parse_context_t *context)
{
    token_t optok  = token(context);
    uint8_t opcode = optok - 100;
    next(context);

    uint16_t addr = 0;
    if ((optok == TOK_JMP) || (optok == TOK_JPC) || (optok == TOK_CAL))
    {
        emit8(opcode);

        // expect label or absolute address
        if (token(context) == TOK_INTEGER)
        {
            emit8(context->lex.lit & 0x0FF);
            emit8((context->lex.lit >> 8) & 0x0FF);
            context->emitaddress++;
        }
        else if (token(context) == TOK_LABEL)
        {
            //TODO: lookup the address
            emit8(0);
            emit8(0);
            context->emitaddress++;            
        }
        else
        {
            // error
            parse_error(context, "expected integer or label\n");
            return false;
        }
    }
    else if (optok == TOK_HALT)
    {
        emit8(opcode);
        emit8(0);
        emit8(0);
        context->emitaddress++;
    }    
    else if (optok == TOK_LIT)
    {
        emit8(opcode);

        if (token(context) != TOK_INTEGER)
        {
            // error
            parse_error(context, "expected integer in literal\n");
            return false;
        }

        emit8(context->lex.lit & 0x0FF);
        emit8((context->lex.lit >> 8) & 0x0FF);
        context->emitaddress++;
    }
    else if ((optok == TOK_LOD) || (optok == TOK_STO))
    {
        if (token(context) != TOK_INTEGER)
        {
            parse_error(context, "Expected integer level or aluop after opcode\n");
            return false;
        }

        opcode |= (context->lex.lit << 4);
        emit8(opcode);

        next(context);        
        if (token(context) != TOK_INTEGER)
        {
            // error
            parse_error(context, "expected integer\n");
            return false;
        }
        emit8(context->lex.lit & 0x0FF);
        emit8((context->lex.lit >> 8) & 0x0FF);
        context->emitaddress++;
    }
    else
    {
        // alu operations
        if (optok == TOK_RET)
        {
            emit8(0x01);    //opr
            emit8(0x00);
            emit8(0x00);
        }
        else if (optok == TOK_NEG)
        {
            emit8(0x01);    //opr
            emit8(0x01);
            emit8(0x00);            
        }
        else if (optok == TOK_ADD)
        {
            emit8(0x01);    //opr
            emit8(0x02);
            emit8(0x00);
        }
        else if (optok == TOK_SUB)
        {
            emit8(0x01);    //opr
            emit8(0x03);
            emit8(0x00);            
        }
        else if (optok == TOK_MUL)
        {
            emit8(0x01);    //opr
            emit8(0x04);
            emit8(0x00);            
        }
        else if (optok == TOK_DIV)
        {
            emit8(0x01);    //opr
            emit8(0x05);
            emit8(0x00);
        }
        else if (optok == TOK_ODD)
        {
            emit8(0x01);    //opr
            emit8(0x06);
            emit8(0x00);            
        }
        else if (optok == TOK_EQU)
        {
            emit8(0x01);    //opr
            emit8(0x08);
            emit8(0x00);            
        }
        else if (optok == TOK_NEQ)
        {
            emit8(0x01);    //opr
            emit8(0x09);
            emit8(0x00);            
        }
        else if (optok == TOK_LES)
        {
            emit8(0x01);    //opr
            emit8(0x0A);
            emit8(0x00);            
        }
        else if (optok == TOK_LEQ)
        {
            emit8(0x01);    //opr
            emit8(0x0B);
            emit8(0x00);            
        }
        else if (optok == TOK_GRE)
        {
            emit8(0x01);    //opr
            emit8(0x0C);
            emit8(0x00);            
        }
        else if (optok == TOK_GEQ)
        {
            emit8(0x01);    //opr
            emit8(0x0D);
            emit8(0x00);            
        }
        else if (optok == TOK_READ)
        {
            emit8(0x01);    //opr
            emit8(0x0E);
            emit8(0x00);            
        }
        else if (optok == TOK_WRITE)
        {
            emit8(0x01);    //opr
            emit8(0x0F);
            emit8(0x00);            
        } 
        else
        {
            parse_error(context, "undefined opcode\n");
            return false;
        }       
    }
    next(context);
    return true;
}

bool parseLine(parse_context_t *context)
{
    // skip empty lines
    if (context->lex.curtok == TOK_EOL)
    {
        next(context);
        return true;
    }

    while((context->lex.curtok != TOK_EOL) && (context->lex.curtok != TOK_EOF))
    {
        if (context->lex.curtok >= 100)
        {
            if (!parse_instruction(context))
                return false;
        }
        else if (context->lex.curtok == TOK_LABEL)
        {
            printf("label ");
            for(uint16_t i=0; i<context->lex.toklen; i++)
                putchar(context->lex.tokstr[i]);
            printf(" at 0x%04X\n", context->emitaddress);
            next(context);
        }
        else
        {
            // error
            parse_error(context, "unexpected token\n");
            return false;
        }
    }
    return true;
}

bool parse(const char *src)
{
    parse_context_t context;
    lex_init(&context.lex, src);
    lex_next(&context.lex);
    context.emitaddress = 0;

    while(context.lex.curtok != TOK_EOF)
    {
        if (!parseLine(&context))
            return false;
    }

    printf("Produced %d bytes\n", context.emitaddress);

    return true;
}
