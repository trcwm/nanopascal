#pragma once

#include <stdint.h>
extern "C"
{
    #include "vm.h"
}

class VMWrapper
{
public:
    VMWrapper();
    virtual ~VMWrapper();

    /** reset cpu / warm boot*/
    void reset();

    /** clear memory and reset */
    void clear();

    /** load code into the VM and reset + halt */
    void load(const uint8_t *code, uint16_t bytes);

    constexpr bool isHalted() const
    {
        return m_halted;
    }

    void halt(bool halt)
    {
        m_halted = halt;
    }

    bool step();

    vm_context_t *m_context;

protected:
    bool m_halted;
};
