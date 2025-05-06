/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#pragma once

#if !defined(__VENOM_SYMTABLE)
#define __VENOM_SYMTABLE

#include "venom/ast.h"

#include "libromano/hashmap.h"

VENOM_CPP_ENTER

typedef enum
{
     VSymType_Module,
     VSymType_Class,
     VSymType_Function,
     VSymType_Variable,
} VSymType;

typedef enum 
{
     VSymScopeType_Global,
     VSymScopeType_Module,
     VSymScopeType_Class,
     VSymScopeType_Function,
     VSymScopeType_Comprehension,
     VSymScopeType_Lambda,
} VSymScopeType;

struct VSymScope {
     VSymScopeType type;
     struct VSymScope* parent;
     VASTNode* ast_node;
     HashMap* symbols;
     Vector* children;
};

typedef struct VSymScope VSymScope;

typedef struct
{
     VSymType type;
} VSym;

typedef struct
{
     VSym base;
     VASTNode* definition;
} VSym_Module;


typedef struct
{
     VSym base;
     VASTNode* definition;
     VASTNode* parent;
} VSym_Class;

typedef struct
{
     VSym base;
     VASTNode* definition;
     VASTNode* parent;
} VSym_Function;

typedef struct
{
     VSym base;
     VSymScopeType scope_type;
     VASTNode* initial_value;
     VASTNode* first_scope;
     VASTNode* last_scope;
} VSym_Variable;

#define VSYM_CAST(__type__, __sym__)                                                              \
     ((__type__*)((__sym__)->type == VSymType_##__type__ ? (__sym__) : NULL))

typedef struct
{
     VSymScope module;
} VSymTable;

VENOM_API void v_symtable_new(VSymTable* symtable);

VENOM_API void v_symtable_destroy(VSymTable* symtable);

VENOM_API void v_symtable_collect(VSymTable* symtable, VAST* ast);

VENOM_API void v_symtable_resolve(VSymTable* symtable, VAST* ast);

VENOM_API VSym* v_symtable_find(VSymTable* symtable, VSymScope* scope, const char* symbol);

VENOM_API void v_symtable_debug(VSymTable* symtable);

/* VSymScope functions */

VENOM_API void v_symscope_new(VSymScope* scope, VSymScopeType type, VSymScope* parent);

VENOM_API void v_symscope_add_symbol(VSymScope* scope, VASTSymbol* symbol);

VENOM_API void v_symscope_destroy(VSymScope* scope);

VENOM_CPP_END

#endif /* !defined(__VENOM_SYMTABLE) */
