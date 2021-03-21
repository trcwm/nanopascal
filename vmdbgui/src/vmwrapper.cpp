#include <stdlib.h>
#include <memory.h>
#include "vmwrapper.h"

VMWrapper::VMWrapper()
{
    m_context = new vm_context_t;
    m_context->mem = nullptr;
    m_context->memsize  = 0;
    m_context->dstack = nullptr;
    clear();    
}

VMWrapper::~VMWrapper()
{
    delete m_context;
}

void VMWrapper::reset()
{
    m_context->pc = 0;
    m_context->t  = 0;
    m_context->b  = 1;
    m_context->inscount = 0;
    m_halted = true;
}

void VMWrapper::clear()
{
    if (m_context->mem != nullptr)
        delete m_context->mem;

    if (m_context->dstack != nullptr)
        vm_free(m_context);

    vm_init(m_context, nullptr, 0);
}

void VMWrapper::load(const uint8_t *code, uint16_t bytes)
{
    clear();
    m_context->mem = new uint8_t[bytes];
    m_context->memsize = bytes;
    memcpy(m_context->mem, code, bytes);
    reset();
}

bool VMWrapper::step()
{
    return vm_execute(m_context);
}