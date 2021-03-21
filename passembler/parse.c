/*

    Parser for p-code assembler
    N.A. Moseley (c) 2021

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "symtbl.h"
#include "parse.h"
#include "fixuptbl.h"

typedef struct 
{
    lex_context_t lex;
    symtbl_t      symtbl;
    fixtbl_t      fixtbl;
    uint16_t      emitaddress;   ///< address of next emitted instruction

    uint8_t       *code;
    uint16_t      codelen;
} parse_context_t;

void emit_ins(parse_context_t *context, 
    const uint8_t  opcode,
    const uint16_t imm16)
{
    context->code[context->emitaddress*3  ] = opcode;
    context->code[context->emitaddress*3+1] = imm16 & 0xFF;
    context->code[context->emitaddress*3+2] = (imm16 >> 8) & 0xFF;
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
    if ((optok == TOK_JMP) || (optok == TOK_JPC))
    {
        // expect label or absolute address
        if (token(context) == TOK_INTEGER)
        {
            emit_ins(context, opcode, context->lex.lit);
            context->emitaddress++;
        }
        else if (token(context) == TOK_LABEL)
        {
            sym_t *label = sym_lookup(&context->symtbl, 
                context->lex.tokstr, 
                context->lex.toklen);

            if (label == NULL)
            {
                // label not found, add to fixup list!
                fix_add(&context->fixtbl, context->lex.tokstr, context->lex.toklen,
                    context->emitaddress);

                emit_ins(context, opcode, 0);
            }
            else
            {
                emit_ins(context, opcode, label->address);
            }
            context->emitaddress++;            
        }
        else
        {
            // error
            parse_error(context, "expected integer or label\n");
            return false;
        }
    }
    else if (optok == TOK_CAL)
    {
        // expect level 
        if (!token(context) != TOK_INTEGER)
        {
            parse_error(context, "CAL level expected\n");
        }

        uint8_t level = (context->lex.lit) & 0xF;
        opcode |= (level << 4);

        next(context);

        // expect label or absolute address
        if (token(context) == TOK_INTEGER)
        {
            emit_ins(context, opcode, context->lex.lit);
            context->emitaddress++;
        }
        else if (token(context) == TOK_LABEL)
        {
            sym_t *label = sym_lookup(&context->symtbl, 
                context->lex.tokstr, 
                context->lex.toklen);

            if (label == NULL)
            {
                // label not found, add to fixup list!
                fix_add(&context->fixtbl, context->lex.tokstr, context->lex.toklen,
                    context->emitaddress);

                emit_ins(context, opcode, 0);
            }
            else
            {
                emit_ins(context, opcode, label->address);
            }
            context->emitaddress++;            
        }
        else
        {
            // error
            parse_error(context, "expected integer or label\n");
            return false;
        }        
    }
    else if ((optok == TOK_HALT) || (optok == TOK_OUTINT) || (optok == TOK_ININT) || (optok == TOK_OUTCHAR))
    {
        emit_ins(context, opcode, 0);
        context->emitaddress++;
    }    
    else if ((optok == TOK_LIT) || (optok == TOK_INT))
    {
        if (token(context) != TOK_INTEGER)
        {
            // error
            parse_error(context, "expected integer in literal\n");
            return false;
        }

        emit_ins(context, opcode, context->lex.lit);
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

        next(context);        
        if (token(context) != TOK_INTEGER)
        {
            // error
            parse_error(context, "expected integer\n");
            return false;
        }
        emit_ins(context, opcode, context->lex.lit);
        context->emitaddress++;
    }
    else
    {
        // alu operations
        if (optok == TOK_RET)
        {
            emit_ins(context, 0x01, 0x00); // opr
            context->emitaddress++;
        }
        else if (optok == TOK_NEG)
        {
            emit_ins(context, 0x01, 0x01); // opr
            context->emitaddress++;       
        }
        else if (optok == TOK_ADD)
        {
            emit_ins(context, 0x01, 0x02); // opr
            context->emitaddress++;
        }
        else if (optok == TOK_SUB)
        {
            emit_ins(context, 0x01, 0x03); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_MUL)
        {
            emit_ins(context, 0x01, 0x04); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_DIV)
        {
            emit_ins(context, 0x01, 0x05); // opr
            context->emitaddress++;
        }
        else if (optok == TOK_ODD)
        {
            emit_ins(context, 0x01, 0x06); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_EQU)
        {
            emit_ins(context, 0x01, 0x08); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_NEQ)
        {
            emit_ins(context, 0x01, 0x09); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_LES)
        {
            emit_ins(context, 0x01, 0x0A); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_LEQ)
        {
            emit_ins(context, 0x01, 0x0B); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_GRE)
        {
            emit_ins(context, 0x01, 0x0C); // opr
            context->emitaddress++; 
        }
        else if (optok == TOK_GEQ)
        {
            emit_ins(context, 0x01, 0x0D); // opr
            context->emitaddress++; 
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
            // enter label into the symbol table
            sym_add(&context->symtbl, context->lex.tokstr, 
                context->lex.toklen, context->emitaddress);
                
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
    sym_init(&context.symtbl);
    fix_init(&context.fixtbl);

    context.codelen = 4096;
    context.code = malloc(context.codelen);
    
    context.emitaddress = 0;

    while(context.lex.curtok != TOK_EOF)
    {
        if (!parseLine(&context))
            return false;
    }

    printf("; Label table:\n");
    sym_dump(&context.symtbl);

    printf("; Fixup table:\n");
    fix_dump(&context.fixtbl);

    printf("; Produced %d bytes\n", context.emitaddress);

    // do fixups
    for(uint16_t i=0; i<context.fixtbl.Nentries; i++)
    {
        fixup_t *fix = &context.fixtbl.fixups[i];
        sym_t *s = sym_lookup(&context.symtbl, fix->name, fix->namelen);
        if (s == NULL)
        {
            printf("Error: cannot find symbol for fixup!\n");
            return false;
        }
        else
        {
            context.code[3*fix->address+1] = s->address & 0xFF;
            context.code[3*fix->address+2] = (s->address >> 8) & 0xFF;
        }
    }

    printf("; Program words:\n");
    for(uint16_t i=0; i<context.emitaddress; i++)
    {
        uint16_t imm = context.code[3*i+1] | (context.code[3*i+2] << 8);
        printf("0x%04x\t0x%02x 0x%04x\n", i, context.code[3*i], imm);
    }

    FILE *cfile = fopen("code.bin","wb");
    fwrite(context.code, 3, context.emitaddress, cfile);
    fclose(cfile);

    return true;
}
