/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#pragma once

#if !defined(__VENOM_TYPE)
#define __VENOM_TYPE

#include "venom/venom.h"

VENOM_CPP_ENTER

/* TODO: add more types */

typedef enum
{
     VType_Unknown,
     VType_None,
     VType_Int,
     VType_Float,
     VType_Bool,
     VType_String,
     VType_Bytes,
     VType_List,
     VType_Tuple,
     VType_Dict,
     VType_Set,
     VType_UserClass,
     VType_Object,
} VType;

VENOM_API VType v_string_to_type(char* start);

VENOM_API const char* v_type_to_string(VType type);

VENOM_CPP_END

#endif /* !defined(__VENOM_TYPE) */