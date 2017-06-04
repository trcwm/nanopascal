/*

  Virtual machine type declarations for Micro Pascal VM
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/


#include "vmtypes.h"



uint16_t getBigEndian(uint8_t v1, uint8_t v2)
{
    uint16_t v = ((uint16_t)v1<<8) | (uint16_t)v2;
    return v;
}

uint16_t getLittleEndian(uint8_t v1, uint8_t v2)
{
    uint16_t v = ((uint16_t)v2<<8) | (uint16_t)v1;
    return v;
}

void VM::disassemble(const std::vector<uint8_t> &bytecode, bool bigEndian)
{
    uint16_t (*getWord)(uint8_t v1, uint8_t v2);

    if (bigEndian)
    {
        getWord = &getBigEndian;
    }
    else
    {
        getWord = &getLittleEndian;
    }

    size_t idx = 0;
    while(idx < bytecode.size())
    {
        switch(bytecode[idx])
        {
        case VM_ADD:
            printf("%04X ADD\n", idx);
            idx++;
            break;
        case VM_SUB:
            printf("%04X ADD\n", idx);
            idx++;
            break;
        case VM_DIV:
            printf("%04X DIV\n", idx);
            idx++;
            break;
        case VM_MUL:
            printf("%04X MUL\n", idx);
            idx++;
            break;
        case VM_INC:
            printf("%04X INC\n", idx);
            idx++;
            break;
        case VM_DEC:
            printf("%04X DEC\n", idx);
            idx++;
            break;
        case VM_DONE:
            printf("%04X DONE\n", idx);
            idx++;
            break;
        case VM_NEG:
            printf("%04X NEG\n", idx);
            idx++;
            break;
        case VM_NOT:
            printf("%04X NOT\n", idx);
            idx++;
            break;
        case VM_CL:
            printf("%04X CL\n", idx);
            idx++;
            break;
        case VM_CG:
            printf("%04X CG\n", idx);
            idx++;
            break;
        case VM_CLE:
            printf("%04X CLE\n", idx);
            idx++;
            break;
        case VM_CGE:
            printf("%04X CGE\n", idx);
            idx++;
            break;
        case VM_CEQ:
            printf("%04X CEQ\n", idx);
            idx++;
            break;
        case VM_CNE:
            printf("%04X CNE\n", idx);
            idx++;
            break;
        case VM_JMP:
            printf("%04X JMP %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_LOAD:
            printf("%04X LOAD %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_STOREP:
            printf("%04X STOREP %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_RESERVE:
            printf("%04X RESERVE %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_LIT:
            printf("%04X LIT %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_JNZ:
            printf("%04X JNZ %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_JZ:
            printf("%04X JZ %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_WRITE:
            printf("%04X WRITE\n", idx);
            idx++;
            break;
        case VM_LOADARG:
            printf("%04X LOADARG %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_CALL:
            printf("%04X CALL %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_RET:
            printf("%04X RET\n\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        case VM_LEAVE:
            printf("%04X LEAVE %04X\n", idx, getWord(bytecode[idx+1], bytecode[idx+2]));
            idx+=3;
            break;
        default:
            printf("%04X UNKNOWN (%d)\n", idx, bytecode[idx]);
            idx++;
            break;
        }
    } // while
}
