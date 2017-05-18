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
    SymbolInfo() : m_type(TYPE_NONE), m_global(false), m_constant(false)
    {
    }

    std::string m_name;
    enum SymType {TYPE_NONE=0, TYPE_UINT16, TYPE_CHAR, TYPE_STRING} m_type;
    bool    m_global;
    bool    m_constant;

    /** absolute address of constant or string.
        relative address (to base pointer) of local
        variable */
    int32_t m_address;
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

    /** lookup a symbol. Returns NULL if not found */
    const SymbolInfo* lookupSymbol(const std::string &name) const
    {
        const size_t N = m_symbols.size();
        for(size_t i=0; i<N; i++)
        {
            if (m_symbols[i].m_name == name)
            {
                return &(m_symbols[i]);
            }

        }
        return NULL;
    }

    /** add a symbol to the symbol table */
    void addSymbol(const std::string &name, SymbolInfo::SymType stype, bool global = false)
    {
        SymbolInfo s;
        s.m_name = name;
        s.m_type = stype;

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
