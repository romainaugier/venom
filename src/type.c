/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/type.h"

#include <string.h>

VType v_string_to_type(char* start)
{
     if(strcmp("typing.", start) == 0)
     {
          start += 7;
     }

     /* When parsing, UserClass will be determined by the symtable */

     if(strcmp(start, "int"))
          return VType_Int;
     if(strcmp(start, "float"))
          return VType_Float;
     if(strcmp(start, "str"))
          return VType_String;
     if(strcmp(start, "bool"))
          return VType_Bool;
     if(strcmp(start, "List"))
          return VType_List;
     if(strcmp(start, "Tuple"))
          return VType_Tuple;
     if(strcmp(start, "Dict"))
          return VType_Dict;
     if(strcmp(start, "Set"))
          return VType_Set;

     return VType_Unknown;
}

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