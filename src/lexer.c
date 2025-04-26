/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/lexer.h"

#include "libromano/hashmap.h"
#include "libromano/logger.h"
#include "libromano/stack_no_alloc.h"

#define PY_LEX_ERROR 0xFFFFFFFF
#define INDENT_SIZE_UNDEFINED 0xFFFFFFFF

const char* token_kind_names[] = {
               "Identifier",
               "Keyword",
               "Literal",
               "Operator",
               "Delimiter",
               "Newline",
               "Indent",
               "Dedent",
};

const char* token_kind_to_string(uint32_t token_type)
{
     if(token_type >= 8)
     {
          return "Unknown";
     }

     return token_kind_names[token_type];
}

const char* v_keyword_to_string(VKeyword keyword)
{
     switch(keyword)
     {
          case VKeyword_False:
               return "false";
          case VKeyword_Await:
               return "await";
          case VKeyword_Else:
               return "else";
          case VKeyword_Import:
               return "import";
          case VKeyword_Pass:
               return "pass";
          case VKeyword_None:
               return "none";
          case VKeyword_Break:
               return "break";
          case VKeyword_Except:
               return "except";
          case VKeyword_In:
               return "in";
          case VKeyword_Raise:
               return "raise";
          case VKeyword_True:
               return "true";
          case VKeyword_Class:
               return "class";
          case VKeyword_Finally:
               return "finally";
          case VKeyword_Is:
               return "is";
          case VKeyword_Return:
               return "return";
          case VKeyword_And:
               return "and";
          case VKeyword_Continue:
               return "continue";
          case VKeyword_For:
               return "for";
          case VKeyword_Lambda:
               return "lambda";
          case VKeyword_Try:
               return "try";
          case VKeyword_As:
               return "as";
          case VKeyword_Def:
               return "def";
          case VKeyword_From:
               return "from";
          case VKeyword_Nonlocal:
               return "nonlocal";
          case VKeyword_While:
               return "while";
          case VKeyword_Assert:
               return "assert";
          case VKeyword_Del:
               return "del";
          case VKeyword_Global:
               return "global";
          case VKeyword_Not:
               return "not";
          case VKeyword_With:
               return "with";
          case VKeyword_Async:
               return "async";
          case VKeyword_Elif:
               return "elif";
          case VKeyword_If:
               return "if";
          case VKeyword_Or:
               return "or";
          case VKeyword_Yield:
               return "yield";
          case VKeyword_Unknown:
               return "unknown";
          default:
               return "unknown";
     }
}

const char* v_operator_to_string(VOperator operator)
{
     switch(operator)
     {
          case VOperator_Addition:
               return "Addition";
          case VOperator_Subtraction:
               return "Subtraction";
          case VOperator_Multiplication:
               return "Multiplication";
          case VOperator_Division:
               return "Division";
          case VOperator_Modulus:
               return "Modulus";
          case VOperator_Exponentiation:
               return "Exponentiation";
          case VOperator_FloorDivision:
               return "FloorDivision";
          case VOperator_Assign:
               return "Assign";
          case VOperator_AdditionAssign:
               return "AdditionAssign";
          case VOperator_SubtractionAssign:
               return "SubtractionAssign";
          case VOperator_MultiplicationAssign:
               return "MultiplicationAssign";
          case VOperator_DivisionAssign:
               return "DivisionAssign";
          case VOperator_ModulusAssign:
               return "ModulusAssign";
          case VOperator_FloorDivisionAssign:
               return "FloorDivisionAssign";
          case VOperator_ExponentiationAssign:
               return "ExponentiationAssign";
          case VOperator_BitwiseAndAssign:
               return "BitwiseAndAssign";
          case VOperator_BitwiseOrAssign:
               return "BitwiseOrAssign";
          case VOperator_BitwiseXorAssign:
               return "BitwiseXorAssign";
          case VOperator_BitwiseLShiftAssign:
               return "BitwiseLShiftAssign";
          case VOperator_BitwiseRShiftAssign:
               return "BitwiseRShiftAssign";
          case VOperator_BitwiseAnd:
               return "BitwiseAnd";
          case VOperator_BitwiseOr:
               return "BitwiseOr";
          case VOperator_BitwiseXor:
               return "BitwiseXor";
          case VOperator_BitwiseNot:
               return "BitwiseNot";
          case VOperator_BitwiseLShift:
               return "BitwiseLShift";
          case VOperator_BitwiseRShift:
               return "BitwiseRShift";
          case VOperator_ComparatorEquals:
               return "ComparatorEquals";
          case VOperator_ComparatorNotEquals:
               return "ComparatorNotEquals";
          case VOperator_ComparatorGreaterThan:
               return "ComparatorGreaterThan";
          case VOperator_ComparatorLessThan:
               return "ComparatorLessThan";
          case VOperator_ComparatorGreaterEqualsThan:
               return "ComparatorGreaterEqualsThan";
          case VOperator_ComparatorLessEqualsThan:
               return "ComparatorLessEqualsThan";
          case VOperator_LogicalAnd:
               return "LogicalAnd";
          case VOperator_LogicalOr:
               return "LogicalOr";
          case VOperator_LogicalNot:
               return "LogicalNot";
          case VOperator_IdentityIs:
               return "IdentityIs";
          case VOperator_IdentityIsNot:
               return "IdentityIsNot";
          case VOperator_MembershipIn:
               return "MembershipIn";
          case VOperator_MembershipNotIn:
               return "MembershipNotIn";
          case VOperator_Unknown:
               return "Unknown";
          default:
               return "unknown";
     }
}

VENOM_FORCE_INLINE bool is_letter(unsigned int c)
{
     return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

VENOM_FORCE_INLINE bool is_digit(unsigned int c)
{
     return c >= '0' && c <= '9';
}

VENOM_FORCE_INLINE bool is_identifier_start(unsigned int c)
{
     return is_letter(c) || c == '_';
}

VENOM_FORCE_INLINE bool is_identifier(unsigned int c)
{
     return is_letter(c) || is_digit(c) || c == '_';
}

VENOM_FORCE_INLINE bool is_delimiter(unsigned int c)
{
     return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' ||
            c == ':' || c == '.' || c == ';' || c == '@' || c == '-' || c == '>';
}

VENOM_FORCE_INLINE bool is_operator_start(unsigned int c)
{
     return c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' || c == '^' ||
            c == '>' || c == '<' || c == '=' || c == '!' || c == '%' || c == 'a' || c == 'i' ||
            c == 'n' || c == 'o';
}

VENOM_FORCE_INLINE bool is_binary_operator(unsigned int c)
{
     return c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' || c == '^' ||
            c == '>' || c == '<' || c == '%';
}

VENOM_FORCE_INLINE bool is_unary_operator(unsigned int c)
{
     return c == '!';
}

VENOM_FORCE_INLINE bool is_assignment_operator(unsigned int c)
{
     return c == '=';
}

VENOM_FORCE_INLINE bool is_string_literal_prefix(unsigned int c)
{
     return c == 'r' || c == 'u' || c == 'R' || c == 'U' || c == 'f' || c == 'F';
}

VENOM_FORCE_INLINE bool is_numeric_literal_start(unsigned int c)
{
     return is_digit(c);
}

VENOM_FORCE_INLINE bool is_int_literal_start(unsigned int c)
{
     return is_digit(c);
}

VENOM_FORCE_INLINE bool is_int_literal(unsigned int c)
{
     return is_digit(c);
}

VENOM_FORCE_INLINE bool is_float_literal_start(unsigned int c)
{
     return is_digit(c);
}

VENOM_FORCE_INLINE bool is_float_literal(unsigned int c)
{
     return is_digit(c) || c == '.';
}

VENOM_FORCE_INLINE uint32_t is_logical_operator(char* c)
{
     if(strncmp(c, "or", 2) == 0)
     {
          return 2;
     }
     else if(strncmp(c, "and", 3) == 0 || strncmp(c, "not", 3) == 0)
     {
          return 3;
     }

     return 0;
}

VENOM_FORCE_INLINE uint32_t is_identity_operator(char* c)
{
     if(strncmp(c, "is", 2) == 0)
     {
          return strncmp(c + 2, " not", 4) == 0 ? 6 : 2;
     }

     return 0;
}

VENOM_FORCE_INLINE uint32_t is_membership_operator(char* c)
{
     if(strncmp(c, "in", 2) == 0)
     {
          return 2;
     }
     else if(strncmp(c, "not in", 6) == 0)
     {
          return 6;
     }

     return 0;
}

VENOM_FORCE_INLINE uint32_t is_operator(char* c)
{
     if(is_binary_operator(*c))
     {
          uint32_t length = 1;
          c++;

          while(is_binary_operator(*c) || is_assignment_operator(*c))
          {
               c++;
               length++;
          }

          return length;
     }
     else if(is_assignment_operator(*c))
     {
          if(is_assignment_operator(*(c + 1)))
          {
               return 2;
          }

          return 1;
     }
     else if(is_unary_operator(*c))
     {
          if(is_assignment_operator(*(c + 1)))
          {
               return 2;
          }

          return PY_LEX_ERROR;
     }

     return 0;
}

VENOM_FORCE_INLINE uint32_t is_string_literal_start(char* c)
{
     if(is_string_literal_prefix(*c) && *(c + 1) != '\0')
     {
          if(*(c + 1) == '"' || *(c + 1) == '\'')
          {
               return 2;
          }
          else if(is_string_literal_prefix(*(c + 1)))
          {
               if(*(c + 2) != '\0' && (*(c + 2) == '"' || *(c + 2) == '\''))
               {
                    return 3;
               }
               else
               {
                    return 0;
               }
          }

          return 0;
     }
     else if(*c == '"' || *c == '\'')
     {
          if(*(c + 1) != '\0' && (*(c + 1) == '"' || *(c + 1) == '\''))
          {
               if(*(c + 2) != '\0' && (*(c + 2) == '"' || *(c + 2) == '\''))
               {
                    return 3;
               }
          }

          return 1;
     }

     return 0;
}

VENOM_FORCE_INLINE uint32_t consume_string_literal(char* c, uint32_t* position, uint32_t* line)
{
     uint32_t length = 0;

     while(*c != '\0' && (*c != '"' && *c != '\''))
     {
          (*position)++;
          c++;
          length++;

          if(*c == '\n')
          {
               *position = 0;
               (*line)++;
          }
     }

     return length;
}

VENOM_FORCE_INLINE uint32_t consume_int_literal(char* c)
{
     uint32_t length = 0;

     while(*c != '\0' && is_digit(*c))
     {
          c++;
          length++;
     }

     return length;
}

VENOM_FORCE_INLINE uint32_t consume_float_literal(char* c)
{
     uint32_t length = 0;
     bool found_dot = false;

     while(*c != '\0')
     {
          if(*c == '.')
          {
               if(found_dot)
               {
                    break;
               }

               found_dot = true;
               c++;
               length++;
          }
          else if(is_digit(*c))
          {
               c++;
               length++;
          }
          else
          {
               break;
          }
     }

     return found_dot ? length : 0;
}

HashMap* g_keywords_map = NULL;
HashMap* g_delimiters_map = NULL;
HashMap* g_operators_map = NULL;

void v_lexer_maps_init()
{
     logger_log_debug("Initialiazing Lexer maps");

     if(g_keywords_map == NULL)
     {
          g_keywords_map = hashmap_new(256);
     }

     uint32_t value = (uint32_t)VKeyword_False;
     hashmap_insert(g_keywords_map, "False", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Await;
     hashmap_insert(g_keywords_map, "await", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Else;
     hashmap_insert(g_keywords_map, "else", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Import;
     hashmap_insert(g_keywords_map, "import", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Pass;
     hashmap_insert(g_keywords_map, "pass", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_None;
     hashmap_insert(g_keywords_map, "None", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Break;
     hashmap_insert(g_keywords_map, "break", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Except;
     hashmap_insert(g_keywords_map, "except", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_In;
     hashmap_insert(g_keywords_map, "in", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Raise;
     hashmap_insert(g_keywords_map, "raise", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_True;
     hashmap_insert(g_keywords_map, "True", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Class;
     hashmap_insert(g_keywords_map, "class", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Finally;
     hashmap_insert(g_keywords_map, "finally", 7, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Is;
     hashmap_insert(g_keywords_map, "is", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Return;
     hashmap_insert(g_keywords_map, "return", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_And;
     hashmap_insert(g_keywords_map, "and", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Continue;
     hashmap_insert(g_keywords_map, "continue", 8, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_For;
     hashmap_insert(g_keywords_map, "for", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Lambda;
     hashmap_insert(g_keywords_map, "lambda", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Try;
     hashmap_insert(g_keywords_map, "try", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_As;
     hashmap_insert(g_keywords_map, "as", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Def;
     hashmap_insert(g_keywords_map, "def", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_From;
     hashmap_insert(g_keywords_map, "from", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Nonlocal;
     hashmap_insert(g_keywords_map, "nonlocal", 8, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_While;
     hashmap_insert(g_keywords_map, "while", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Assert;
     hashmap_insert(g_keywords_map, "assert", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Del;
     hashmap_insert(g_keywords_map, "del", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Global;
     hashmap_insert(g_keywords_map, "global", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Not;
     hashmap_insert(g_keywords_map, "not", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_With;
     hashmap_insert(g_keywords_map, "with", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Async;
     hashmap_insert(g_keywords_map, "async", 5, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Elif;
     hashmap_insert(g_keywords_map, "elif", 4, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_If;
     hashmap_insert(g_keywords_map, "if", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Or;
     hashmap_insert(g_keywords_map, "or", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VKeyword_Yield;
     hashmap_insert(g_keywords_map, "yield", 5, &value, sizeof(uint32_t));

     if(g_delimiters_map == NULL)
     {
          g_delimiters_map = hashmap_new(256);
     }

     value = VDelimiter_LParen;
     hashmap_insert(g_delimiters_map, "(", 1, &value, sizeof(uint32_t));
     value = VDelimiter_RParen;
     hashmap_insert(g_delimiters_map, ")", 1, &value, sizeof(uint32_t));
     value = VDelimiter_LBracket;
     hashmap_insert(g_delimiters_map, "[", 1, &value, sizeof(uint32_t));
     value = VDelimiter_RBracket;
     hashmap_insert(g_delimiters_map, "]", 1, &value, sizeof(uint32_t));
     value = VDelimiter_LBrace;
     hashmap_insert(g_delimiters_map, "{", 1, &value, sizeof(uint32_t));
     value = VDelimiter_RBrace;
     hashmap_insert(g_delimiters_map, "}", 1, &value, sizeof(uint32_t));
     value = VDelimiter_Comma;
     hashmap_insert(g_delimiters_map, ",", 1, &value, sizeof(uint32_t));
     value = VDelimiter_Colon;
     hashmap_insert(g_delimiters_map, ":", 1, &value, sizeof(uint32_t));
     value = VDelimiter_Dot;
     hashmap_insert(g_delimiters_map, ".", 1, &value, sizeof(uint32_t));
     value = VDelimiter_SemiColon;
     hashmap_insert(g_delimiters_map, ";", 1, &value, sizeof(uint32_t));
     value = VDelimiter_At;
     hashmap_insert(g_delimiters_map, "@", 1, &value, sizeof(uint32_t));
     value = VDelimiter_RightArrow;
     hashmap_insert(g_delimiters_map, "->", 2, &value, sizeof(uint32_t));

     if(g_operators_map == NULL)
     {
          g_operators_map = hashmap_new(256);
     }

     value = (uint32_t)VOperator_Addition;
     hashmap_insert(g_operators_map, "+", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_Subtraction;
     hashmap_insert(g_operators_map, "-", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_Multiplication;
     hashmap_insert(g_operators_map, "*", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_Division;
     hashmap_insert(g_operators_map, "/", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_Modulus;
     hashmap_insert(g_operators_map, "%", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_Exponentiation;
     hashmap_insert(g_operators_map, "**", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_FloorDivision;
     hashmap_insert(g_operators_map, "//", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_Assign;
     hashmap_insert(g_operators_map, "=", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_AdditionAssign;
     hashmap_insert(g_operators_map, "+=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_SubtractionAssign;
     hashmap_insert(g_operators_map, "-=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_MultiplicationAssign;
     hashmap_insert(g_operators_map, "*=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_DivisionAssign;
     hashmap_insert(g_operators_map, "/=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ModulusAssign;
     hashmap_insert(g_operators_map, "%=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_FloorDivisionAssign;
     hashmap_insert(g_operators_map, "//=", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ExponentiationAssign;
     hashmap_insert(g_operators_map, "**=", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseAndAssign;
     hashmap_insert(g_operators_map, "&=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseOrAssign;
     hashmap_insert(g_operators_map, "|=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseXorAssign;
     hashmap_insert(g_operators_map, "^=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseLShiftAssign;
     hashmap_insert(g_operators_map, "<<=", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseRShiftAssign;
     hashmap_insert(g_operators_map, ">>=", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseAnd;
     hashmap_insert(g_operators_map, "&", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseOr;
     hashmap_insert(g_operators_map, "|", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseXor;
     hashmap_insert(g_operators_map, "^", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseNot;
     hashmap_insert(g_operators_map, "~", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseLShift;
     hashmap_insert(g_operators_map, "<<", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_BitwiseRShift;
     hashmap_insert(g_operators_map, ">>", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ComparatorEquals;
     hashmap_insert(g_operators_map, "==", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ComparatorNotEquals;
     hashmap_insert(g_operators_map, "!=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ComparatorGreaterThan;
     hashmap_insert(g_operators_map, ">", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ComparatorLessThan;
     hashmap_insert(g_operators_map, "<", 1, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ComparatorGreaterEqualsThan;
     hashmap_insert(g_operators_map, ">=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_ComparatorLessEqualsThan;
     hashmap_insert(g_operators_map, "<=", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_LogicalAnd;
     hashmap_insert(g_operators_map, "and", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_LogicalOr;
     hashmap_insert(g_operators_map, "or", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_LogicalNot;
     hashmap_insert(g_operators_map, "not", 3, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_IdentityIs;
     hashmap_insert(g_operators_map, "is", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_IdentityIsNot;
     hashmap_insert(g_operators_map, "is not", 6, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_MembershipIn;
     hashmap_insert(g_operators_map, "in", 2, &value, sizeof(uint32_t));
     value = (uint32_t)VOperator_MembershipNotIn;
     hashmap_insert(g_operators_map, "not in", 6, &value, sizeof(uint32_t));
}

uint32_t v_lexer_maps_get_keyword(const char* word, const uint32_t word_size)
{
     VENOM_ASSERT(g_keywords_map != NULL, "Keywords map has not been initialized");

     void* keyword = hashmap_get(g_keywords_map, (const void*)word, word_size, NULL);

     if(keyword == NULL)
     {
          return VKeyword_Unknown;
     }

     return *((uint32_t*)keyword);
}

uint32_t v_lexer_maps_get_delimiter(const char* word, const uint32_t word_size)
{
     VENOM_ASSERT(g_delimiters_map != NULL, "Delimiters map has not been initialized");

     void* delimiter = hashmap_get(g_delimiters_map, (const void*)word, word_size, NULL);

     if(delimiter == NULL)
     {
          return VDelimiter_Unknown;
     }

     return *((uint32_t*)delimiter);
}

uint32_t v_lexer_maps_get_operator(const char* word, const uint32_t word_size)
{
     VENOM_ASSERT(g_operators_map != NULL, "Operators map has not been initialized");

     void* operator= hashmap_get(g_operators_map, (const void*)word, word_size, NULL);

     if(operator== NULL)
     {
          return VOperator_Unknown;
     }

     return *((uint32_t*)operator);
}

void v_lexer_maps_release()
{
     logger_log_debug("Releasing Lexer maps");

     if(g_keywords_map != NULL)
     {
          hashmap_free(g_keywords_map);
          g_keywords_map = NULL;
     }

     if(g_delimiters_map != NULL)
     {
          hashmap_free(g_delimiters_map);
          g_delimiters_map = NULL;
     }

     if(g_operators_map != NULL)
     {
          hashmap_free(g_operators_map);
          g_operators_map = NULL;
     }
}

void v_lexer_token_debug(VToken* token)
{
     printf("%s: %.*s\n", token_kind_to_string(token->kind), (int)token->length, token->start);
}

#define MAX_INDENT_DEPTH 128

bool v_lexer_lex(char* buffer, Vector* tokens)
{
     char* s = buffer;

     uint32_t indent_stack[MAX_INDENT_DEPTH];
     size_t indent_stack_ptr = 0;

     indent_stack[0] = 0;
     indent_stack_ptr = 1;

     uint32_t indent_size = INDENT_SIZE_UNDEFINED;
     uint32_t indent_level = 0;
     uint32_t position = 1;
     uint32_t line = 1;

     while(*s != '\0')
     {
          const uint32_t string_literal_prefix = is_string_literal_start(s);

          if(string_literal_prefix != 0)
          {
               s += string_literal_prefix;
               position += string_literal_prefix;

               char* start = s;
               uint32_t length = consume_string_literal(s, &position, &line);

               s += length;

               VToken token = {start, length, VTokenKind_Literal, VLiteral_String, position, line};

               vector_push_back(tokens, &token);

               while(*s != '\0' && (*s == '"' || *s == '\''))
               {
                    s++;
                    position++;
               }
          }
          else if(is_numeric_literal_start(*s))
          {
               char* start = s;
               uint32_t length = consume_float_literal(s);

               if(length > 0)
               {
                    VToken token = {start,
                                    length,
                                    VTokenKind_Literal,
                                    VLiteral_Float,
                                    position,
                                    line};

                    vector_push_back(tokens, &token);

                    s += length;
                    position += length;
                    continue;
               }

               length = consume_int_literal(s);

               if(length > 0)
               {
                    VToken token = {start,
                                    length,
                                    VTokenKind_Literal,
                                    VLiteral_Integer,
                                    position,
                                    line};

                    vector_push_back(tokens, &token);

                    s += length;
                    position += length;
                    continue;
               }
          }
          else if(is_identifier_start(*s))
          {
               char* start = s;
               uint32_t length = 1;

               s++;
               position++;

               while(*s != '\0' && is_identifier(*s))
               {
                    s++;
                    position++;
                    length++;
               }

               uint32_t value = v_lexer_maps_get_keyword(start, length);

               if(value != VKeyword_Unknown)
               {
                    VToken token = {start,
                                    length,
                                    VTokenKind_Keyword,
                                    value,
                                    position - length,
                                    line};

                    vector_push_back(tokens, &token);
                    continue;
               }

               value = v_lexer_maps_get_operator(start, length);

               if(value != VOperator_Unknown)
               {
                    VToken token = {start,
                                    length,
                                    VTokenKind_Operator,
                                    value,
                                    position - length,
                                    line};

                    vector_push_back(tokens, &token);
                    continue;
               }

               VToken token = {start, length, VTokenKind_Identifier, 0, position - length, line};

               vector_push_back(tokens, &token);
          }
          else if(is_delimiter(*s))
          {
               char* start = s;
               uint32_t length = 1;

               s++;
               position++;

               if(*start == '-')
               {
                    while(*s != '\0' && is_delimiter(*s))
                    {
                         s++;
                         position++;
                         length++;
                    }
               }

               uint32_t value = v_lexer_maps_get_delimiter(start, length);

               if(value == VDelimiter_Unknown)
               {
                    s -= length;
                    position -= length;
                    goto label_is_operator;
               }

               VToken token = {start, length, VTokenKind_Delimiter, value, position, line};

               vector_push_back(tokens, &token);
          }
          else if(is_operator_start(*s))
          {
          label_is_operator:;
               char* start = s;
               uint32_t length = is_operator(s);

               if(length > 0)
               {
                    position += length;

                    uint32_t value = v_lexer_maps_get_operator(start, length);

                    if(value == VOperator_Unknown || length == PY_LEX_ERROR)
                    {
                         logger_log_error("Syntax Error: Invalid operator: %.*s",
                                          (int)length,
                                          start);
                         logger_log_error("Line %u, Position %u", line, position);
                         return false;
                    }

                    VToken token = {start, length, VTokenKind_Operator, value, position, line};

                    vector_push_back(tokens, &token);

                    s += length;
                    continue;
               }
          }
          else if(*s == '\n')
          {
               VToken token = {NULL, 0, VTokenKind_Newline, 0, position, line};

               vector_push_back(tokens, &token);

               s++;

               line++;
               position = 0;

               while(*s != '\0' && *s == '\n')
               {
                    VToken newline_token = {NULL, 0, VTokenKind_Newline, 0, position, line};

                    vector_push_back(tokens, &newline_token);

                    s++;
                    line++;
               }

               uint32_t line_indent = 0;
               char* indent_start = s;

               while(*s != '\0' && *s == ' ')
               {
                    line_indent++;
                    s++;
                    position++;
               }

               if(*s == '\n' || *s == '0' || *s == '#')
               {
                    continue;
               }

               uint32_t current_indent_level = indent_stack[indent_stack_ptr - 1];

               if(line_indent > current_indent_level)
               {
                    if(indent_stack_ptr >= MAX_INDENT_DEPTH)
                    {
                         logger_log_error("Indentation Error: Maximum indentation depth exceeded "
                                          "(%d)",
                                          MAX_INDENT_DEPTH);
                         logger_log_error("Line %u, Position %u", line, position - line_indent);
                         return false;
                    }

                    indent_stack[indent_stack_ptr] = line_indent;
                    indent_stack_ptr++;

                    VToken indent_token = {indent_start,
                                           line_indent,
                                           VTokenKind_Indent,
                                           0,
                                           position - line_indent,
                                           line};

                    vector_push_back(tokens, &indent_token);
               }
               else if(line_indent < current_indent_level)
               {
                    while(indent_stack_ptr > 1 && indent_stack[indent_stack_ptr - 1] > line_indent)
                    {
                         indent_stack_ptr--;

                         VToken dedent_token = {indent_start,
                                                0,
                                                VTokenKind_Dedent,
                                                0,
                                                position - line_indent,
                                                line};
                         vector_push_back(tokens, &dedent_token);

                         current_indent_level = indent_stack[indent_stack_ptr - 1];
                    }

                    if(indent_stack[indent_stack_ptr - 1] != line_indent)
                    {
                         logger_log_error("Indentation Error: Unindent does not match any outer "
                                          "indentation level");
                         logger_log_error("Line %u, Position %u (Expected indent %u, got %u)",
                                          line,
                                          position - line_indent,
                                          indent_stack[indent_stack_ptr - 1],
                                          line_indent);
                         return false;
                    }
               }

               indent_level = line_indent;
          }
          else
          {
               s++;
               position++;
          }
     }

     VToken eof_token = {NULL, 0, VTokenKind_EOF, 0, position, line};

     vector_push_back(tokens, &eof_token);

     return true;
}