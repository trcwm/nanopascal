/*

  P-code generator for Micro Pascal compiler
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#ifndef pcodegenerator_h
#define pcodegenerator_h

#include <vector>
#include "parser.h"
#include "vmtypes.h"
#include "symboltable.h"

class PCodeGenerator
{
public:
    PCodeGenerator(bool targetIsBigEndian, bool debug);

    bool process(const AST::ASTNode *head, std::vector<uint8_t> &code);

protected:
    struct fixup_t
    {
        uint16_t    m_location; // location within the byte stream
        uint16_t    m_labelID;  // label ID
    };

    /** emit a 1-byte instruction into the instruction stream */
    void VMEmitInstruction(VM::VMInstruction a);

    /** emit a 2-byte instruction into the instruction stream */
    void VMEmitInstruction(VM::VMInstruction a, uint8_t param);

    /** emit a 3-byte instruction into the instruction stream */
    void VMEmitInstruction(VM::VMInstruction a, uint16_t param);

    /** emit a 3-byte instruction into the instruction stream,
        where the last two bytes are an address defined by a
        label. The address actually emitted is 0x0000 as
        the label might not have a known address.
        The label is added tot the fixup vector so the
        0x0000 address can be patched after all the
        VM code has been emitted.
    */
    void VMEmitInstructionWithLabel(VM::VMInstruction a, uint16_t labelID);

    /** emit a label and return its ID, address is unknown.
        This will create a label for future reference.
    */
    uint16_t VMEmitLabel();

    /** set the address of the label */
    void VMSetLabelAddress(uint16_t ID, uint16_t address);

    /** get the current emit address pointer */
    uint16_t getCurrentEmitAddress() const
    {
        return m_code->size();
    }

    /** throws a std::runtime error */
    void error(const std::string &str);

    // emit instructions for a node
    void processNode(const AST::ASTNode *node);

    // special compound statements handled here:
    void handleForStatement(const AST::ASTNode *forNode);
    void handleIfStatement(const AST::ASTNode *ifNode);
    void handleWriteFunction(const AST::ASTNode *writeNode);

    // substitute the label address in the VM
    // instruction stream based on the
    // collected fixup data
    void doFixups();

    SymbolTable::ScopedTable    *m_curSymScope;         // pointer to symboltable of current scope
    std::vector<uint8_t>        *m_code;                // vector that will receive VM binary
    std::vector<fixup_t>        m_fixups;
    std::vector<uint16_t>       m_labels;               // vector containing label addresses

    bool                        m_targetIsBigEndian;    // set to true if target is big endian
    bool                        m_emitReserve;          // if true, reserved mem is emitted at the
                                                        // start of a new progblock
    bool                        m_debug;
};

#endif
