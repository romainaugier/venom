/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/symtable.h"

#include "libromano/stack_no_alloc.h"

/*******************************/
/* SymTable functions */

/* SymTable Construction */

void v_symtable_new(VSymTable* symtable)
{
    VENOM_ASSERT(symtable != NULL, "SymTable cannot be NULL");

    memset(symtable, 0, sizeof(VSymTable));

    v_symscope_new(&symtable->module, VSymScopeType_Module, NULL);

    symtable->module.symbols = hashmap_new(128);
    symtable->module.children = vector_new(128, sizeof(VSymScope));
}

void v_symtable_destroy(VSymTable* symtable)
{
    if(symtable != NULL)
    {
        v_symscope_destroy(&symtable->module);
    }
}

void v_symtable_collect(VSymTable* symtable, VAST* ast)
{
    stack_init(VSymScope*, scopes, 256);

    stack_push(scopes, &symtable->module);

    v_symscope_new(stack_top(scopes), VSymScopeType_Global, NULL);

    VENOM_NOT_IMPLEMENTED;
}

void v_symtable_resolve(VSymTable* symtable, VAST* ast)
{
    VENOM_NOT_IMPLEMENTED;
}

VSym* v_symtable_find(VSymTable* symtable, VSymScope* scope, const char* symbol)
{
    VENOM_NOT_IMPLEMENTED;
    return NULL;
}

/* SymTable Debugging */

VENOM_FORCE_INLINE void print_indent(const uint32_t indent_level)
{
    for(uint32_t i = 0; i < indent_level; i++)
    {
        printf("  ");
    }
}

void v_sym_debug(VSym* sym, const uint32_t indent_level)
{
    if(sym == NULL)
    {
        return;
    }

    print_indent(indent_level);

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

const char* v_scope_type_to_string(VSymScopeType type)
{
    switch(type)
    {
        case VSymScopeType_Global:
            return "Global";
        case VSymScopeType_Module:
            return "Module";
        case VSymScopeType_Class:
            return "Class";
        case VSymScopeType_Function:
            return "Function";
        case VSymScopeType_Comprehension:
            return "Comprehension";
        case VSymScopeType_Lambda:
            return "Lambda";
        default:
            return "Unknown";
    }
}

void v_scope_debug(VSymScope* scope, const uint32_t indent_level)
{
    if(scope == NULL)
    {
        return;
    }

    print_indent(indent_level);

    printf("%s Scope\n", v_scope_type_to_string(scope->type));

    if(scope->symbols != NULL)
    {
        print_indent(indent_level);
        printf("Symbols:\n");

        HashMapIterator it = 0;
        void* key;
        void* value;

        while(hashmap_iterate(scope->symbols, &it, &key, NULL, &value, NULL))
        {
            v_sym_debug((VSym*)value, indent_level + 1);
        }
    }
}

void v_symtable_debug(VSymTable* symtable)
{
    v_scope_debug(&symtable->module, 0);
}

/* VSymScope functions */

void v_symscope_new(VSymScope* scope, VSymScopeType type, VSymScope* parent)
{
    memset(scope, 0, sizeof(VSymScope));

    scope->type = type;
    scope->parent = parent;
}

void v_symscope_destroy(VSymScope* scope)
{
    if(scope != NULL)
    {
        if(scope->symbols != NULL)
        {
            hashmap_free(scope->symbols);
        }

        if(scope->children != NULL)
        {
            vector_free_with_dtor(scope->children, v_symscope_destroy);
        }

        memset(scope, 0, sizeof(VSymScope));
    }
}