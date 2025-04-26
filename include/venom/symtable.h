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
     VSymType_Variable,
     VSymType_Function,
     VSymType_Class,
} VSymType;

typedef enum 
{
     VSymScopeType_Global,
     VSymScopeType_Local,
} VSymScopeType;

typedef struct
{
     VSymType type;
} VSym;

typedef struct
{
     VSymType type;
     VSymScopeType scope_type;
     VASTNode* initial_value;
     VASTNode* scope;
} VSym_Variable;

typedef struct
{
     VSymType type;
} VSym_Function;

typedef struct
{
     VSymType type;
} VSym_Class;

#define VSYM_CAST(__type__, __sym__)                                                              \
     ((__type__*)((__sym__)->type == VSymType_##__type__ ? (__sym__) : NULL))

typedef struct
{
     HashMap* symbols;
} VSymTable;

VENOM_API void v_symtable_new(VSymTable* symtable);

VENOM_API void v_symtable_destroy(VSymTable* symtable);

VENOM_API void v_symtable_debug(VSymTable* symtable);

VENOM_CPP_END

#endif /* !defined(__VENOM_SYMTABLE) */
