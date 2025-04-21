/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/type.h"

const char* v_type_to_string(VType type)
{
    switch(type)
    {
        case VType_Unknown:
            return "Unknown";
        case VType_Int:
            return "Int";
        case VType_Float:
            return "Float";
        case VType_String:
            return "String";
        case VType_Bool:
            return "Bool";
        case VType_None:
            return "None";
        case VType_List:
            return "List";
        case VType_Tuple:
            return "Tuple";
        case VType_Dict:
            return "Dict";
        case VType_Set:
            return "Set";
        case VType_UserClass:
            return "UserClass";
        default:
            return "Unknown";
    }
}