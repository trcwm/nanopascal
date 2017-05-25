/*

  P-code generator for Micro Pascal compiler
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#include <sstream>
#include "pcodegenerator.h"

PCodeGenerator::PCodeGenerator(bool targetIsBigEndian, bool debug) : m_targetIsBigEndian(targetIsBigEndian),
    m_debug(debug)
{
}

/** depth first */
bool PCodeGenerator::process(const AST::ASTNode *head, std::vector<uint8_t> &code)
{
    code.clear();
    m_labels.clear();   // clear all the fixup labels

    if (m_debug)
    {
        printf("Running PCodeGenerator...\n");
    }

    try
    {
        // VM code output array and setup the global symbol table.
        m_code = &code;
        m_curSymScope = new SymbolTable::ScopedTable(0);
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

    if (node->m_type == AST::NODE_IFSTATEMENT)
    {
        handleIfStatement(node);
        return;
    }

    if (node->m_type == AST::NODE_WRITE)
    {
        handleWriteFunction(node);
        return;
    }

    // do stuff upon entering the node
    SymbolTable::ScopedTable *newScope;
    SymbolTable::SymbolInfo  *procinfo;
    switch(node->m_type)
    {
    case AST::NODE_BLOCK:
        /* generate a stack frame because we can have
           new local variables
        */

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
            // loop through all the local variables/constants and save space for them
            // note: function/procedure args already have space allocated for them
            //       by the caller; no need to take them into account!
            VMEmitInstruction(VM::VM_RESERVE, (uint16_t)m_curSymScope->calculateLocalStorageBytes());
            if (m_debug)
            {
                printf("Local storage size: %d bytes\n", m_curSymScope->calculateLocalStorageBytes());
            }
        }
        break;
    case AST::NODE_PROCDECL:
        // we must generate a new stack frame!
        m_emitReserve = false;

        // store the procedure information in the symbol table!
        procinfo = m_curSymScope->addProcedure(node->m_txt);
        newScope = new SymbolTable::ScopedTable(m_curSymScope);

        // add all the arguments to the new scope
        //for(size_t i=0; i<procinfo->m_args.size(); i++)
        //{
        //    newScope->addArgument(procinfo->m_args[i].m_name,
        //                          procinfo->m_args[i].m_type,
        //                          false);
        //}
        m_curSymScope = newScope;
        if (m_debug) printf("PROCDECL %s\n", node->m_txt.c_str());
        break;

    default:
        ;
    }

    for(uint32_t i=0; i<node->m_children.size(); i++)
    {
        processNode(node->m_children[i]);
    }

    // do stuff upon leaving the node
    const SymbolTable::SymbolInfo *info;
    SymbolTable::ScopedTable *tmpScope;
    switch(node->m_type)
    {
    case AST::NODE_BLOCK:
        /* generate a stack frame because we have
           new local variables */
        //if (m_debug) printf("Clean up stack frame\n");
        //m_curSymScope = m_curSymScope->getParent();

        break;
    case AST::NODE_LITERALINTEGER:
        if (m_debug) printf("LIT $%04X (%d)\n", node->m_integer, node->m_integer);
        VMEmitInstruction(VM::VM_LIT, static_cast<uint16_t>(node->m_integer));
        break;
    case AST::NODE_LOGIC:
    case AST::NODE_ARITH:
        switch(node->m_optype)
        {
        case AST::ANODE_COPY:
            if (m_debug) printf("COPY\n");
            break;
        case AST::ANODE_MINUS:
            VMEmitInstruction(VM::VM_SUB);
            if (m_debug) printf("-\n");
            break;
        case AST::ANODE_PLUS:
            VMEmitInstruction(VM::VM_ADD);
            if (m_debug) printf("+\n");
            break;
        case AST::ANODE_MUL:
            VMEmitInstruction(VM::VM_MUL);
            if (m_debug) printf("*\n");
            break;
        case AST::ANODE_DIV:
            VMEmitInstruction(VM::VM_DIV);
            if (m_debug) printf("/\n");
            break;
        case AST::ANODE_UMINUS:
            VMEmitInstruction(VM::VM_NEG);
            if (m_debug) printf("U-\n");
            break;
        case AST::LNODE_EQUAL:
            if (m_debug) printf("==\n");
            VMEmitInstruction(VM::VM_CEQ);
            break;
        case AST::LNODE_GREATER:
            if (m_debug) printf(">\n");
            VMEmitInstruction(VM::VM_CG);
            break;
        case AST::LNODE_GREATEROREQUAL:
            if (m_debug) printf(">=\n");
            VMEmitInstruction(VM::VM_CGE);
            break;
        case AST::LNODE_LESS:
            if (m_debug) printf("<\n");
            VMEmitInstruction(VM::VM_CL);
            break;
        case AST::LNODE_LESSOREQUAL:
            if (m_debug) printf("<=\n");
            VMEmitInstruction(VM::VM_CLE);
            break;
        case AST::LNODE_NOT:
            if (m_debug) printf("NOT\n");
            VMEmitInstruction(VM::VM_NOT);
            break;
        case AST::LNODE_NOTEQUAL:
            if (m_debug) printf("!=\n");
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
            //FIXME: emit labels for the store command and do fixup later
            if (m_debug) printf("STOREP %d (%s)\n", 0, node->m_txt.c_str());
            VMEmitInstruction(VM::VM_STOREP, static_cast<uint16_t>(0));
        }
        break;
    case AST::NODE_USEVARINTEGER:        
        // lookup the variable
        info = m_curSymScope->lookupSymbol(node->m_txt.c_str());
        if (info == NULL)
        {
            std::stringstream ss;
            ss << "Error symbol " << node->m_txt << "  does not exist!";
            error(ss.str());
        }
        else
        {
            if (info->m_arg)
            {
                // FIXME: emit labels and calculate offsets later
                if (m_debug) printf("LOADARG %d (%s)\n", 0, node->m_txt.c_str());
                VMEmitInstruction(VM::VM_LOADARG, static_cast<uint16_t>(0));
            }
            else
            {
                // FIXME: emit labels and calculate offsets later
                if (m_debug) printf("LOAD %d (%s)\n", 0, node->m_txt.c_str());
                VMEmitInstruction(VM::VM_LOAD, static_cast<uint16_t>(0));
            }
        }
        break;
    case AST::NODE_DECLVARINTEGER:
        m_emitReserve = true; // we have local variables!
        m_curSymScope->addIdentifier(node->m_txt, SymbolTable::SymbolInfo::TYPE_UINT16);
        if (m_debug) printf("DECLARE %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_DECLCONSTINTEGER:
        //m_emitReserve = true; // we have local variables!
        //m_curSymScope->addSymbol(node->m_txt, node->m_string);
        //if (m_debug) printf("DECLARE %s\n", node->m_txt.c_str());
        if (m_debug) printf("LIT $%04X (%d dec)\n", node->m_integer, node->m_integer);
        VMEmitInstruction(VM::VM_LIT, static_cast<uint16_t>(node->m_integer));
        break;
    case AST::NODE_ARGDECL:
        m_curSymScope->addArgument(node->m_txt, SymbolTable::SymbolInfo::TYPE_UINT16, false);
        //m_curSymScope->addArgVariable(node->m_txt, SymbolTable::SymbolInfo::TYPE_UINT16);
        if (m_debug) printf("ARGDECL %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_PROCDECL:
        // the procedure has already been added to the
        // scope above when entering the node
        // now we must destroy the local
        // procedure scope
        tmpScope = m_curSymScope;
        m_curSymScope = m_curSymScope->getParent();
        delete tmpScope;
        if (m_debug) printf("PROCDECL(end) %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_CONSTSTRING:
        // push the address of the string onto the
        // stack.
        // lookup the variable offset
        info = m_curSymScope->lookupSymbol(node->m_txt.c_str());
        if (info == NULL)
        {
            std::stringstream ss;
            ss << "Error constant string " << node->m_txt << "  does not exist!";
            error(ss.str());
        }
        else
        {
            //FIXME: emit labels for address
            //if (m_debug) printf("LIT $%04X (%d dec); const string %s\n", info->m_address, info->m_address, node->m_txt.c_str());
            if (m_debug) printf("LIT $%04X (%d dec); const string %s\n", 0, 0, node->m_txt.c_str());
        }
        break;
    case AST::NODE_CALL:
        if (m_debug) printf("CALL %s\n", node->m_txt.c_str());
        break;
        // nodes that do nothing..
    case AST::NODE_PROGBLOCK:
    case AST::NODE_CONSTDECL:
    case AST::NODE_STATEMENT:
    case AST::NODE_EXPR:
    case AST::NODE_VARDECL:
        break;
    default:
        if (m_debug) printf("Unprocessed AST node: %d\n", node->m_type);
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

    //FIXME: add labels
    if (m_debug) printf("STOREP %d (%s)\n", 0, forNode->m_txt.c_str());
    VMEmitInstruction(VM::VM_STOREP, static_cast<uint16_t>(0));

    // now we insert a FOR loop label and check
    // if we haven't reached the end of the loop
    if (m_debug) printf("LOOPSTART:\n");
    uint16_t loopStartID = VMEmitLabel();
    VMSetLabelAddress(loopStartID, getCurrentEmitAddress());

    // FIXME: labels
    if (m_debug) printf("LOAD %d (%s)\n", 0, forNode->m_txt.c_str());    // push loop variable onto stack
    VMEmitInstruction(VM::VM_LOAD, static_cast<uint16_t>(0));

    processNode(forNode->m_children[1]);            // loop terminating value/expression


    if (forNode->m_integer < 0)
    {
        if (m_debug) printf(">=\n");
        VMEmitInstruction(VM::VM_CGE);
    }
    else
    {
        if (m_debug) printf("<=\n");
        VMEmitInstruction(VM::VM_CLE);
    }

    if (m_debug) printf("JNZ LOOPEXIT\n");
    uint16_t loopExitID = VMEmitLabel();
    VMEmitInstructionWithLabel(VM::VM_JZ, loopExitID); // address to be patched later

    // do the loop body
    processNode(forNode->m_children[2]);

    // update the loop variable
    //FIXME:
    if (m_debug) printf("LOAD %d (%s)\n", 0, forNode->m_txt.c_str());
    VMEmitInstruction(VM::VM_LOAD, static_cast<uint16_t>(0));
    if (forNode->m_integer < 0)
    {
        if (m_debug) printf("DEC\n");
        VMEmitInstruction(VM::VM_DEC);
    }
    else
    {
        if (m_debug) printf("INC\n");
        VMEmitInstruction(VM::VM_INC);
    }

    //FIXME:
    if (m_debug) printf("STOREP %d (%s)\n", 0, forNode->m_txt.c_str());   // store loop variable
    VMEmitInstruction(VM::VM_STOREP, static_cast<uint16_t>(0));

    if (m_debug) printf("JMP LOOPSTART\n");
    VMEmitInstructionWithLabel(VM::VM_JMP, loopStartID);

    if (m_debug) printf("LOOPEXIT:\n");
    VMSetLabelAddress(loopExitID, getCurrentEmitAddress());
}


void PCodeGenerator::handleIfStatement(const AST::ASTNode *ifNode)
{
    // IF statement node has 2 or 3 child nodes:
    // expr, THEN statement and optional ELSE statement.
    //
    // Emit:
    //      expr code
    //      JZ skipthen
    //      statement1 code
    //      JMP ifend
    // skipthen:
    //      statement2 code (optional)
    // ifend:
    //

    processNode(ifNode->m_children[0]); // emit expr code

    uint16_t skipthenID = VMEmitLabel();
    VMEmitInstructionWithLabel(VM::VM_JZ, skipthenID);

    processNode(ifNode->m_children[1]);    // emit statement 1 code

    uint16_t ifendID = VMEmitLabel();
    VMEmitInstructionWithLabel(VM::VM_JMP, ifendID);

    // emit skipthen label
    VMSetLabelAddress(skipthenID, getCurrentEmitAddress());

    // optionally emit statement2
    if (ifNode->m_children.size() > 2)
    {
        processNode(ifNode->m_children[2]);    // emit statement 2 code
    }

    // emit ifend label
    VMSetLabelAddress(ifendID, getCurrentEmitAddress());
}

void PCodeGenerator::handleWriteFunction(const AST::ASTNode *writeNode)
{
    // simply emit a write instruction for every argument
    // for now we'll assume they're all integers
    for(size_t i=0; i<writeNode->m_children.size(); i++)
    {
        // emit argument
        processNode(writeNode->m_children[i]);
        VMEmitInstruction(VM::VM_WRITE);
    }
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
