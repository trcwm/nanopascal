/*

  Symboltable for the Micro Pascal compiler
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

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
        m_global(false),
        m_constant(false),
        m_argument(false)
    {
    }

    ~SymbolInfo()
    {
    }

    std::string m_name;
    std::string m_string;
    enum SymType {TYPE_NONE=0, TYPE_UINT16, TYPE_CHAR,
                  TYPE_STRING, TYPE_PROCEDURE,
                  TYPE_FUNCTION} m_type;

    bool    m_global;
    bool    m_constant;
    bool    m_argument;

    /** absolute address of constant or string.
        relative address (to base pointer) of local
        variable */
    int32_t m_address;

    std::vector<SymbolInfo> m_args;    // function or procedure arguments
};

/** a collection of symbols in one scope */
class ScopedTable
{
public:
    ScopedTable(ScopedTable *parent = NULL)
        : m_parent(parent), m_offset(0)
    {
        if (parent != NULL)
        {
            parent->addChild(this);
        }
    }

    ~ScopedTable()
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

    void addChild(ScopedTable *child)
    {
        m_children.push_back(child);
    }

    /** lookup a symbol. Returns NULL if not found.
        if the scope is not local, only
        procedures, function and global
        variables are returned.
    */
    const SymbolInfo* lookupSymbol(const std::string &name, bool scopeIsLocal = true) const
    {
        const size_t N = m_symbols.size();
        for(size_t i=0; i<N; i++)
        {
            if (m_symbols[i].m_name == name)
            {
                // check if we're searching the local scope
                if (scopeIsLocal)
                {
                    // return everything
                    return &(m_symbols[i]);
                }
                else
                {
                    // return only global vars
                    // functions or  procedures
                    switch(m_symbols[i].m_type)
                    {
                    case SymbolInfo::TYPE_FUNCTION:
                    case SymbolInfo::TYPE_PROCEDURE:
                        return &(m_symbols[i]);
                    }

                    if (m_symbols[i].m_global)
                    {
                        return &(m_symbols[i]);
                    }
                }
            }
        }
        return NULL;
    }

    /** add a variable to the symbol table */
    void addConstant(const std::string &name, SymbolInfo::SymType stype, bool global = false)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = stype;
        s.m_constant = true;

        // automatically calculate the local
        // address/offset if it's not a
        // global variable
        if (!global)
        {
            s.m_address = m_offset;
            switch(stype)
            {
            case SymbolInfo::TYPE_CHAR:
                m_offset++;
                break;
            case SymbolInfo::TYPE_UINT16:
                m_offset+=2;
                break;
            }
        }
        else
        {
            //TODO: handle global variables
            // or constants
        }
        m_symbols.push_back(s);
    }

    /** add a function argument variable to the symbol table */
    void addArgVariable(const std::string &name, SymbolInfo::SymType stype)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = stype;
        s.m_global = false;
        s.m_constant = false;
        s.m_argument = true;

        //FIXME: calculate stack address of
        // argument
#if 0
        s.m_address = m_offset;
        switch(stype)
        {
        case SymbolInfo::TYPE_CHAR:
            m_offset++;
            break;
        case SymbolInfo::TYPE_UINT16:
            m_offset+=2;
            break;
        }
#endif
        m_symbols.push_back(s);
    }

    /** add a variable to the symbol table */
    void addVariable(const std::string &name, SymbolInfo::SymType stype, bool global = false)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = stype;
        s.m_global = global;
        s.m_constant = false;

        // automatically calculate the local
        // address/offset if it's not a
        // global variable
        if (!global)
        {
            s.m_address = m_offset;
            switch(stype)
            {
            case SymbolInfo::TYPE_CHAR:
                m_offset++;
                break;
            case SymbolInfo::TYPE_UINT16:
                m_offset+=2;
                break;
            }
        }
        else
        {
            //TODO: handle global variables
            // or constants
        }
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
        s.m_global = false;
        s.m_constant = false;

        m_symbols.push_back(s);
        return &(m_symbols[m_symbols.size()-1]);
    }

    /** add a function to the symbol table
        and return a pointer to the info object
        so we can add arguments */
    SymbolInfo* addFunction(const std::string &name)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = SymbolInfo::TYPE_FUNCTION;
        s.m_global = false;
        s.m_constant = false;

        m_symbols.push_back(s);
        return &(m_symbols[m_symbols.size()-1]);
    }

    /** add a string constant symbol to the symbol table */
    void addConstStringSymbol(const std::string &name, const std::string &txt, bool global = false)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = SymbolInfo::TYPE_STRING;
        s.m_string = txt;
        s.m_string += '\0'; // add NULL terminator
        s.m_constant = true;

        // automatically calculate the local
        // address/offset if it's not a
        // global variable
        if (!global)
        {
            s.m_address = m_offset;
            m_offset += s.m_string.size();
        }
        else
        {
            //TODO: handle global variables
            // or constants
        }
        m_symbols.push_back(s);
    }

    /** get pointer to the parent scope */
    ScopedTable* getParent() const
    {
        return m_parent;
    }

    /** return the number of bytes that
        need to be reserved for local
        storage. */
    uint16_t getLocalStorageSize() const
    {
        if (m_offset < 0)
            return 0;

        return static_cast<uint16_t>(m_offset);
    }

protected:
    std::vector<SymbolInfo> m_symbols;
    std::vector<ScopedTable*> m_children;
    ScopedTable *m_parent;

    // Offset of first empty memory position
    // for local variable storage
    int32_t     m_offset;
};

} // namespace
#endif
