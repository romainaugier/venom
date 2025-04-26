/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/symtable.h"

/*******************************/
/* SymTable functions */

/* SymTable Construction */

void v_symtable_new(VSymTable* symtable)
{
    VENOM_ASSERT(symtable != NULL, "SymTable cannot be NULL");

    memset(symtable, 0, sizeof(VSymTable));

    symtable->symbols = hashmap_new(128);
}

void v_symtable_destroy(VSymTable* symtable)
{
    if(symtable != NULL)
    {
        if(symtable->symbols != NULL)
        {
            hashmap_free(symtable->symbols);
        }

        memset(symtable, 0, sizeof(VSymTable));
    }
}

/* SymTable Debugging */

void v_sym_debug(VSym* sym)
{
    const VSymType type = sym->type;

    switch (type)
    {
        case VSymType_Class:
        {
            break;
        }

        case VSymType_Function:
        {
            break;
        }

        case VSymType_Variable:
        {
            break;
        }
        
        default:
            break;
    }
}

void v_symtable_debug(VSymTable* symtable)
{
    VENOM_NOT_IMPLEMENTED;
}