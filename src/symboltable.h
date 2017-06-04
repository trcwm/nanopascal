/*

  Symbol table for the Micro Pascal compiler
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

  The symbol table holds information on the variables, constants,
  functions and procedures used by the Pascal program.

  One can look up a symbol by name to check its type,
  size (in bytes) and several attributes, such as whether
  or not the symbol is a constant, etc.

  For functions, the return type is stored.

  For functions and procedures, a list of arguments
  can be returned.




*/

#ifndef symboltable_h
#define symboltable_h

#include <stdint.h>
#include <string>
#include <vector>

namespace SymbolTable
{

/** Information describing a symbol */
struct SymbolInfo
{
    SymbolInfo() : m_type(TYPE_NONE),
        m_constant(false), m_arg(false), m_address(-1)
    {
    }

    ~SymbolInfo()
    {
    }

    std::string     m_name;     // name of variable, function or procedure
    std::string     m_string;   // string constant


    /** For constants, this holds the absolute address of
        the constant in the constant pool. For variables,
        it holds the address relative to the stack frame.
        For arguments, it holds the address relative to the
        stack frame.
        For functions and procedures it holds the call
        address.
        */
    int32_t         m_address;

    enum SymType {TYPE_NONE=0,
                  TYPE_UINT16,
                  TYPE_CHAR,
                  TYPE_STRING,
                  TYPE_PROCEDURE,
                  TYPE_FUNCTION,
                  TYPE_LABEL
                 } m_type;      // type of symbol

    SymType         m_rtype;            // return type for functions

    /** if true, this datatype is a constant,
        i.e. it's immutable. */
    bool    m_constant;                 // constant flag

    /** true if this symbol is a function argument identifier
        in the local scope. */
    bool    m_arg;                      // default = false

    std::vector<SymbolInfo> m_args;     // function or procedure arguments

    /** return the number of bytes that are required
        to store it. This does not work for
        procedures or functions; 0 is returned.
    */
    size_t getSymbolBytes() const
    {
        switch(m_type)
        {
        case TYPE_UINT16:
            return 2;
        case TYPE_CHAR:
            return 1;
        case TYPE_STRING:
            // The NULL terminator should be stored in the
            // m_string for this to work!
            return m_string.size();
        default:
            return 0;
        }
    }
};


/** a collection of symbols in one scope */
class ScopedTable
{
public:
    ScopedTable(ScopedTable *parent = NULL)
        : m_parent(parent), m_localStorageBytes(0)
    {
        if (parent != NULL)
        {
            parent->addChildScope(this);
        }
    }

    virtual ~ScopedTable()
    {
        clear();
    }

    /** clear the symbol table, destroying its children */
    void clear()
    {
        for(uint32_t i=0; i<m_children.size(); i++)
        {
            delete m_children[i];
        }
    }

    /** add scoped table to the list of children */
    void addChildScope(ScopedTable *child)
    {
        m_children.push_back(child);
    }

    /** Lookup a symbol. Returns NULL if not found.
        if the scope is not local, only
        procedures, function and global
        variables are returned.

        If 'searchOnlyLocalScope' is true,
        only the local table is searched. Otherwise,
        the search will go up the scope hierarchy.
    */
    const SymbolInfo* lookupSymbol(const std::string &name, bool searchOnlyLocalScope = false) const
    {
        // we're searching a new stack frame here,
        // so reset the stack-frame-relative address
        // to zero
        const size_t N = m_symbols.size();
        for(size_t i=0; i<N; i++)
        {
            if (m_symbols[i].m_name == name)
            {
                return &(m_symbols[i]);
            }
        }

        // we didn't find the symbol in the local scope
        // so travel up the hierarchy and try to
        // find it there, but only when
        // searchOnlyLocalScope == false
        if (!searchOnlyLocalScope)
        {
            if (m_parent != 0)
            {
                return m_parent->lookupSymbol(name, false);
            }
        }
        return NULL;
    }

    /** add a variable or constant to the symbol table and
        reserve space for it in the local stack frame.
    */
    void addIdentifier(const std::string &name, SymbolInfo::SymType stype, bool constant = false)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = stype;
        s.m_constant = constant;

        // reserve space for the ident in the local stack frame
        s.m_address = m_localStorageBytes;
        m_localStorageBytes += s.getSymbolBytes();
        m_symbols.push_back(s);
    }

    /** add a (local) function argument to the symbol table.
        these variables are not part of the local stack frame
        but rather of the stack space of the caller.
        Therefore, there is no need to increment the
        local storage amount.
    */
    void addArgument(const std::string &name, SymbolInfo::SymType stype, uint32_t address)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = stype;
        s.m_constant = false;
        s.m_arg = true;
        s.m_address = address;
        m_symbols.push_back(s);
    }

    /** add a procedure to the symbol table
        and return a pointer to the info object
        so we can add arguments */
    SymbolInfo* addProcedure(const std::string &name)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = SymbolInfo::TYPE_PROCEDURE;
        s.m_constant = false;
        s.m_address = 0;
        m_symbols.push_back(s);
        return &(m_symbols[m_symbols.size()-1]);
    }

    /** add a function to the symbol table
        and return a pointer to the info object
        so we can add arguments */
    SymbolInfo* addFunction(const std::string &name, uint16_t labelID, SymbolInfo::SymType returnType)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = SymbolInfo::TYPE_FUNCTION;
        s.m_constant = false;
        s.m_rtype = returnType;
        s.m_address = labelID;
        m_symbols.push_back(s);
        return &(m_symbols[m_symbols.size()-1]);
    }

    /** add a string constant symbol to the symbol table.
        The string is stored in a seperate constant pool.
        To access it, we must store the address of it.
    */
    void addConstStringSymbol(const std::string &name, const std::string &txt)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = SymbolInfo::TYPE_STRING;
        s.m_string = txt;
        s.m_string += '\0'; // add NULL terminator
        s.m_constant = true;

        //FIXME: allocate space in the constant pool and
        // find the address!
        s.m_address = 0;
        m_symbols.push_back(s);
    }

    /** get pointer to the parent scope */
    ScopedTable* getParent() const
    {
        return m_parent;
    }

    /** calculate the required space to allocate
        all the local variables */
    size_t getLocalStorageBytes() const
    {
        return m_localStorageBytes;
    }

protected:
    /** required stack space to store local variables
        and constants */
    uint32_t                  m_localStorageBytes;

    std::vector<SymbolInfo>   m_symbols;
    std::vector<ScopedTable*> m_children;
    ScopedTable *m_parent;
};

} // namespace
#endif
