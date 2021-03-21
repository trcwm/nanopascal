#pragma once

#include <stdbool.h>
#include "opcodes.h"

void vm_init(vm_context_t *c, uint8_t *memptr, uint16_t memsize);
void vm_free(vm_context_t *c);
void vm_push(vm_context_t *c, uint16_t v);
bool vm_execute(vm_context_t *c);

