/*

  P-code generator for Micro Pascal compiler
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#include <sstream>
#include "pcodegenerator.h"

PCodeGenerator::PCodeGenerator(bool targetIsBigEndian) : m_targetIsBigEndian(targetIsBigEndian)
{
}

/** depth first */
bool PCodeGenerator::process(const AST::ASTNode *head, std::vector<uint8_t> &code)
{
    m_syms.clear();
    code.clear();
    m_curSymScope = &m_syms;
    m_labels.clear();

    try
    {
        m_code = &code;
        processNode(head);
        doFixups();
    }
    catch(std::runtime_error &e)
    {
        return false;
    }
    return true;
}

void PCodeGenerator::processNode(const AST::ASTNode *node)
{
    /** handle special node, such as FOR statements separately. */

    if (node->m_type == AST::NODE_FORSTATEMENT)
    {
        handleForStatement(node);
        return;
    }

    // do stuff upon entering the node
    SymbolTable::ScopedTable *newScope;
    switch(node->m_type)
    {
    case AST::NODE_BLOCK:
        /* generate a stack frame because we have
           new local variables */
        printf("Create stack frame\n");
        newScope = new SymbolTable::ScopedTable(m_curSymScope);
        m_curSymScope = newScope;

        // reset the reserve flag, as we don't know if we
        // have local variables!
        m_emitReserve = false;
        break;
    case AST::NODE_PROGBLOCK:
        // for the first progblock after the
        // var and const declarations
        // we must reserve the space for
        // the local vars!
        if (m_emitReserve)
        {
            m_emitReserve = false;
            VMEmitInstruction(VM::VM_RESERVE, m_curSymScope->getLocalStorageSize());
            printf("Local storage size: %d bytes\n", m_curSymScope->getLocalStorageSize());
        }
        break;
    }

    for(uint32_t i=0; i<node->m_children.size(); i++)
    {
        processNode(node->m_children[i]);
    }

    // do stuff upon leaving the node
    const SymbolTable::SymbolInfo *info;
    switch(node->m_type)
    {
    case AST::NODE_BLOCK:
        /* generate a stack frame because we have
           new local variables */
        printf("Clean up stack frame\n");
        m_curSymScope = m_curSymScope->getParent();

        break;
    case AST::NODE_LITERALINTEGER:
        printf("LIT $%04X (%d)\n", node->m_integer, node->m_integer);
        VMEmitInstruction(VM::VM_LIT, static_cast<uint16_t>(node->m_integer));
        break;
    case AST::NODE_LOGIC:
    case AST::NODE_ARITH:
        switch(node->m_optype)
        {
        case AST::ANODE_COPY:
            printf("COPY\n");
            break;
        case AST::ANODE_MINUS:
            VMEmitInstruction(VM::VM_SUB);
            printf("-\n");
            break;
        case AST::ANODE_PLUS:
            VMEmitInstruction(VM::VM_ADD);
            printf("+\n");
            break;
        case AST::ANODE_MUL:
            VMEmitInstruction(VM::VM_MUL);
            printf("*\n");
            break;
        case AST::ANODE_DIV:
            VMEmitInstruction(VM::VM_DIV);
            printf("/\n");
            break;
        case AST::ANODE_UMINUS:
            VMEmitInstruction(VM::VM_NEG);
            printf("U-\n");
            break;
        case AST::LNODE_EQUAL:
            printf("==\n");
            VMEmitInstruction(VM::VM_CEQ);
            break;
        case AST::LNODE_GREATER:
            printf(">\n");
            VMEmitInstruction(VM::VM_CG);
            break;
        case AST::LNODE_GREATEROREQUAL:
            printf(">=\n");
            VMEmitInstruction(VM::VM_CGE);
            break;
        case AST::LNODE_LESS:
            printf("<\n");
            VMEmitInstruction(VM::VM_CL);
            break;
        case AST::LNODE_LESSOREQUAL:
            printf("<=\n");
            VMEmitInstruction(VM::VM_CLE);
            break;
        case AST::LNODE_NOT:
            printf("NOT\n");
            VMEmitInstruction(VM::VM_NOT);
            break;
        case AST::LNODE_NOTEQUAL:
            printf("!=\n");
            VMEmitInstruction(VM::VM_CNE);
            break;
        default:
            error("Unknown ARITH or LOGIC node operation!");
            break;
        }
        break;
    case AST::NODE_ASSIGN:
        // lookup the variable offset
        info = m_curSymScope->lookupSymbol(node->m_txt.c_str());
        if (info == NULL)
        {
            std::stringstream ss;
            ss << "Error symbol " << node->m_txt << "  does not exist!";
            error(ss.str());
        }
        else
        {
            printf("STOREP %d (%s)\n", info->m_address, node->m_txt.c_str());
            VMEmitInstruction(VM::VM_STOREP, static_cast<uint16_t>(info->m_address));
        }
        break;
    case AST::NODE_USEVARINTEGER:
        // lookup the variable offset
        info = m_curSymScope->lookupSymbol(node->m_txt.c_str());
        if (info == NULL)
        {
            std::stringstream ss;
            ss << "Error symbol " << node->m_txt << "  does not exist!";
            error(ss.str());
        }
        else
        {
            printf("LOAD %d (%s)\n", info->m_address, node->m_txt.c_str());
            VMEmitInstruction(VM::VM_LOAD, static_cast<uint16_t>(info->m_address));
        }
        break;
    case AST::NODE_DECLVARINTEGER:
        m_emitReserve = true; // we have local variables!
        m_curSymScope->addSymbol(node->m_txt.c_str(), SymbolTable::SymbolInfo::TYPE_UINT16);
        printf("DECLARE %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_CONSTINTEGER:
        // note: we just emit constants in the instruction stream
        // no need to use a local var as the overhead is
        // exactly the same!
        printf("LIT $%04X (%d dec)\n", node->m_integer, node->m_integer);
        VMEmitInstruction(VM::VM_LIT, static_cast<uint16_t>(node->m_integer));
        break;

        // nodes that do nothing..
    case AST::NODE_PROGBLOCK:
    case AST::NODE_CONSTDECL:
    case AST::NODE_STATEMENT:
    case AST::NODE_EXPR:
    case AST::NODE_VARDECL:
        break;
    default:
        printf("Unprocessed AST node: %d\n", node->m_type);
        break;
    }
}

void PCodeGenerator::handleForStatement(const AST::ASTNode *forNode)
{
    const SymbolTable::SymbolInfo *info;

    // first we need to initialize the FOR loop variable
    // by evaluating the FOR loops first expression

    processNode(forNode->m_children[0]);    // initialization expression
    info = m_curSymScope->lookupSymbol(forNode->m_txt.c_str());
    if (info == NULL)
    {
        std::stringstream ss;
        ss << "Error symbol " << forNode->m_txt << "  does not exist!";
        error(ss.str());
    }

    printf("STOREP %d (%s)\n", info->m_address, forNode->m_txt.c_str());
    VMEmitInstruction(VM::VM_STOREP, static_cast<uint16_t>(info->m_address));

    // now we insert a FOR loop label and check
    // if we haven't reached the end of the loop
    printf("LOOPSTART:\n");
    uint16_t loopStartID = VMEmitLabel();
    VMSetLabelAddress(loopStartID, getCurrentEmitAddress());

    printf("LOAD %d (%s)\n", info->m_address, forNode->m_txt.c_str());    // push loop variable onto stack
    VMEmitInstruction(VM::VM_LOAD, static_cast<uint16_t>(info->m_address));

    processNode(forNode->m_children[1]);            // loop terminating value/expression

    // TODO: check if these are correct later
    if (forNode->m_integer < 0)
    {
        printf("<\n");
        VMEmitInstruction(VM::VM_CL);
    }
    else
    {
        printf(">\n");
        VMEmitInstruction(VM::VM_CG);
    }

    printf("JNZ LOOPEXIT\n");
    uint16_t loopExitID = VMEmitLabel();
    VMEmitInstructionWithLabel(VM::VM_JZ, loopExitID); // address to be patched later

    // do the loop body
    processNode(forNode->m_children[2]);

    // update the loop variable
    printf("LOAD %d (%s)\n", info->m_address, forNode->m_txt.c_str());
    VMEmitInstruction(VM::VM_LOAD, static_cast<uint16_t>(info->m_address));
    if (forNode->m_integer < 0)
    {
        printf("DEC\n");
        VMEmitInstruction(VM::VM_DEC);
    }
    else
    {
        printf("INC\n");
        VMEmitInstruction(VM::VM_DEC);
    }

    printf("STOREP %d (%s)\n", info->m_address, forNode->m_txt.c_str());   // store loop variable
    VMEmitInstruction(VM::VM_STOREP, static_cast<uint16_t>(info->m_address));

    printf("JMP LOOPSTART\n");
    VMEmitInstructionWithLabel(VM::VM_JMP, loopStartID);

    printf("LOOPEXIT:\n");
    VMSetLabelAddress(loopExitID, getCurrentEmitAddress());
}

void PCodeGenerator::error(const std::string &str)
{
    printf("%s\n", str.c_str());
    throw std::runtime_error(str.c_str());
}

void PCodeGenerator::VMEmitInstruction(VM::VMInstruction a)
{
    m_code->push_back(static_cast<uint8_t>(a));
}

void PCodeGenerator::VMEmitInstruction(VM::VMInstruction a, uint8_t param)
{
    m_code->push_back(static_cast<uint8_t>(a));
    m_code->push_back(static_cast<uint8_t>(param));
}

void PCodeGenerator::VMEmitInstruction(VM::VMInstruction a, uint16_t param)
{
    m_code->push_back(static_cast<uint8_t>(a));
    if (m_targetIsBigEndian)
    {
        m_code->push_back(static_cast<uint8_t>(param>>8));  // MSB first for HD6309 target!
        m_code->push_back(static_cast<uint8_t>(param));
    }
    else
    {
        m_code->push_back(static_cast<uint8_t>(param));     // LSB first
        m_code->push_back(static_cast<uint8_t>(param>>8));
    }
}

void PCodeGenerator::VMEmitInstructionWithLabel(VM::VMInstruction a, uint16_t labelID)
{
    fixup_t fixup;
    fixup.m_labelID = labelID;
    m_code->push_back(static_cast<uint8_t>(a));
    fixup.m_location = m_code->size();
    m_fixups.push_back(fixup);

    m_code->push_back(0x00);    // address to be patched later
    m_code->push_back(0x00);
}

uint16_t PCodeGenerator::VMEmitLabel()
{
    uint16_t ID = m_labels.size();
    m_labels.push_back(0);
    return ID;
}

void PCodeGenerator::VMSetLabelAddress(uint16_t ID, uint16_t address)
{
    m_labels[ID] = address;
}

void PCodeGenerator::doFixups()
{
    for(size_t i=0; i<m_fixups.size(); i++)
    {
        uint16_t ID = m_fixups[i].m_labelID;            // get the ID of the label
        uint16_t location = m_fixups[i].m_location;     // get the location of the label in the VM stream
        uint16_t address = m_labels[ID];    // get the (VM) address of the label
        // patch it!
        if (m_targetIsBigEndian)
        {
            // MSB first!
            m_code->operator[](location++) = static_cast<uint8_t>(address >> 8);
            m_code->operator[](location) = static_cast<uint8_t>(address);
        }
        else
        {
            // LSB first!
            m_code->operator[](location++) = static_cast<uint8_t>(address);
            m_code->operator[](location) = static_cast<uint8_t>(address >> 8);
        }
    }
}
