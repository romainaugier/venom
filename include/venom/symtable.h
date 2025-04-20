/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#pragma once

#if !defined(__VENOM_SYMTABLE)
#define __VENOM_SYMTABLE

#include "venom/ast.h"

#include "libromano/hashmap.h"

VENOM_CPP_ENTER

enum VSymType {
    VSymType_Variable,
    VSymType_Function,
    VSymType_Class,
};

typedef struct {
    VSymType type;
} VSym;

typedef struct {
    VSymType type;
} VSym_Variable;

typedef struct {
    VSymType type;
} VSym_Function;

typedef struct {
    VSymType type;
} VSym_Class;

typedef struct {
    HashMap* symbols;
} VSymTable;

VENOM_CPP_END

#endif /* !defined(__VENOM_SYMTABLE) */
