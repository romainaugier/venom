/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/lexer.h"

#include "libromano/hashmap.h"
#include "libromano/logger.h"

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
    return c == '(' || 
           c == ')' || 
           c == '[' || 
           c == ']' || 
           c == '{' || 
           c == '}' || 
           c == ',' || 
           c == ':' || 
           c == '.' || 
           c == ';' || 
           c == '@' || 
           c == '-' || 
           c == '>';
}

VENOM_FORCE_INLINE bool is_operator_start(unsigned int c) 
{
    return c == '+' || 
           c == '-' || 
           c == '*' || 
           c == '/' || 
           c == '&' || 
           c == '|' || 
           c == '^' || 
           c == '>' || 
           c == '<' || 
           c == '=' || 
           c == '!' || 
           c == '%' || 
           c == 'a' || 
           c == 'i' || 
           c == 'n' || 
           c == 'o';
}

VENOM_FORCE_INLINE bool is_binary_operator(unsigned int c) 
{
    return c == '+' || 
           c == '-' || 
           c == '*' || 
           c == '/' || 
           c == '&' || 
           c == '|' || 
           c == '^' || 
           c == '>' || 
           c == '<' || 
           c == '%';
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
    return c == 'r' || 
           c == 'u' || 
           c == 'R' || 
           c == 'U' || 
           c == 'f' || 
           c == 'F';
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

void lexer_maps_init() 
{
    logger_log_debug("Initialiazing Lexer maps");

    if(g_keywords_map == NULL)
    {
        g_keywords_map = hashmap_new(256);
    }

    uint32_t value = (uint32_t)PyKeyword_False; 
    hashmap_insert(g_keywords_map, "False", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Await; 
    hashmap_insert(g_keywords_map, "await", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Else; 
    hashmap_insert(g_keywords_map, "else", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Import; 
    hashmap_insert(g_keywords_map, "import", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Pass; 
    hashmap_insert(g_keywords_map, "pass", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_None; 
    hashmap_insert(g_keywords_map, "None", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Break; 
    hashmap_insert(g_keywords_map, "break", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Except; 
    hashmap_insert(g_keywords_map, "except", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_In; 
    hashmap_insert(g_keywords_map, "in", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Raise; 
    hashmap_insert(g_keywords_map, "raise", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_True; 
    hashmap_insert(g_keywords_map, "True", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Class; 
    hashmap_insert(g_keywords_map, "class", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Finally; 
    hashmap_insert(g_keywords_map, "finally", 7, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Is; 
    hashmap_insert(g_keywords_map, "is", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Return; 
    hashmap_insert(g_keywords_map, "return", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_And; 
    hashmap_insert(g_keywords_map, "and", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Continue; 
    hashmap_insert(g_keywords_map, "continue", 8, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_For; 
    hashmap_insert(g_keywords_map, "for", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Lambda; 
    hashmap_insert(g_keywords_map, "lambda", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Try; 
    hashmap_insert(g_keywords_map, "try", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_As; 
    hashmap_insert(g_keywords_map, "as", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Def; 
    hashmap_insert(g_keywords_map, "def", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_From; 
    hashmap_insert(g_keywords_map, "from", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Nonlocal; 
    hashmap_insert(g_keywords_map, "nonlocal", 8, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_While; 
    hashmap_insert(g_keywords_map, "while", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Assert; 
    hashmap_insert(g_keywords_map, "assert", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Del; 
    hashmap_insert(g_keywords_map, "del", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Global; 
    hashmap_insert(g_keywords_map, "global", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Not; 
    hashmap_insert(g_keywords_map, "not", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_With; 
    hashmap_insert(g_keywords_map, "with", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Async; 
    hashmap_insert(g_keywords_map, "async", 5, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Elif; 
    hashmap_insert(g_keywords_map, "elif", 4, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_If; 
    hashmap_insert(g_keywords_map, "if", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Or; 
    hashmap_insert(g_keywords_map, "or", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyKeyword_Yield; 
    hashmap_insert(g_keywords_map, "yield", 5, &value, sizeof(uint32_t));

    if(g_delimiters_map == NULL)
    {
        g_delimiters_map = hashmap_new(256);
    }

    value = PyDelimiter_LParen; 
    hashmap_insert(g_delimiters_map, "(", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_RParen; 
    hashmap_insert(g_delimiters_map, ")", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_LBracket; 
    hashmap_insert(g_delimiters_map, "[", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_RBracket; 
    hashmap_insert(g_delimiters_map, "]", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_LBrace; 
    hashmap_insert(g_delimiters_map, "{", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_RBrace; 
    hashmap_insert(g_delimiters_map, "}", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_Comma; 
    hashmap_insert(g_delimiters_map, ",", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_Colon; 
    hashmap_insert(g_delimiters_map, ":", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_Dot; 
    hashmap_insert(g_delimiters_map, ".", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_SemiColon; 
    hashmap_insert(g_delimiters_map, ";", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_At; 
    hashmap_insert(g_delimiters_map, "@", 1, &value, sizeof(uint32_t));
    value = PyDelimiter_RightArrow; 
    hashmap_insert(g_delimiters_map, "->", 2, &value, sizeof(uint32_t));

    if(g_operators_map == NULL)
    {
        g_operators_map = hashmap_new(256);
    }

    value = (uint32_t)PyOperator_Addition; 
    hashmap_insert(g_operators_map, "+", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Subtraction; 
    hashmap_insert(g_operators_map, "-", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Multiplication; 
    hashmap_insert(g_operators_map, "*", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Division; 
    hashmap_insert(g_operators_map, "/", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Modulus; 
    hashmap_insert(g_operators_map, "%", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Exponentiation; 
    hashmap_insert(g_operators_map, "**", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Floor_division; 
    hashmap_insert(g_operators_map, "//", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_Assign; 
    hashmap_insert(g_operators_map, "=", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_AdditionAssign; 
    hashmap_insert(g_operators_map, "+=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_SubtractionAssign; 
    hashmap_insert(g_operators_map, "-=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_MultiplicationAssign; 
    hashmap_insert(g_operators_map, "*=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_DivisionAssign; 
    hashmap_insert(g_operators_map, "/=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ModulusAssign; 
    hashmap_insert(g_operators_map, "%=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_FloorDivisionAssign; 
    hashmap_insert(g_operators_map, "//=", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ExponentiationAssign; 
    hashmap_insert(g_operators_map, "**=", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseAndAssign; 
    hashmap_insert(g_operators_map, "&=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseOrAssign; 
    hashmap_insert(g_operators_map, "|=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseXorAssign; 
    hashmap_insert(g_operators_map, "^=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseLShiftAssign; 
    hashmap_insert(g_operators_map, "<<=", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseRShiftAssign; 
    hashmap_insert(g_operators_map, ">>=", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseAnd; 
    hashmap_insert(g_operators_map, "&", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseOr; 
    hashmap_insert(g_operators_map, "|", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseXor; 
    hashmap_insert(g_operators_map, "^", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseNot; 
    hashmap_insert(g_operators_map, "~", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseLShift; 
    hashmap_insert(g_operators_map, "<<", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_BitwiseRShift; 
    hashmap_insert(g_operators_map, ">>", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ComparatorEquals; 
    hashmap_insert(g_operators_map, "==", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ComparatorNotEquals; 
    hashmap_insert(g_operators_map, "!=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ComparatorGreaterThan; 
    hashmap_insert(g_operators_map, ">", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ComparatorLessThan; 
    hashmap_insert(g_operators_map, "<", 1, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ComparatorGreaterEqualsThan; 
    hashmap_insert(g_operators_map, ">=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_ComparatorLessEqualsThan; 
    hashmap_insert(g_operators_map, "<=", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_LogicalAnd; 
    hashmap_insert(g_operators_map, "and", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_LogicalOr; 
    hashmap_insert(g_operators_map, "or", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_LogicalNot; 
    hashmap_insert(g_operators_map, "not", 3, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_IdentityIs; 
    hashmap_insert(g_operators_map, "is", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_IdentityIsNot; 
    hashmap_insert(g_operators_map, "is not", 6, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_MembershipIn; 
    hashmap_insert(g_operators_map, "in", 2, &value, sizeof(uint32_t));
    value = (uint32_t)PyOperator_MembershipNotIn; 
    hashmap_insert(g_operators_map, "not in", 6, &value, sizeof(uint32_t));
}

uint32_t lexer_maps_get_keyword(const char* word, const uint32_t word_size)
{
    VENOM_ASSERT(g_keywords_map != NULL, "Keywords map has not been initialized");

    void* keyword = hashmap_get(g_keywords_map, (const void*)word, word_size, NULL);

    if(keyword == NULL)
    {
        return PyKeyword_Unknown;
    }

    return *((uint32_t*)keyword);
}

uint32_t lexer_maps_get_delimiter(const char* word, const uint32_t word_size)
{
    VENOM_ASSERT(g_delimiters_map != NULL, "Delimiters map has not been initialized");

    void* delimiter = hashmap_get(g_delimiters_map, (const void*)word, word_size, NULL);

    if(delimiter == NULL)
    {
        return PyDelimiter_Unknown;
    }

    return *((uint32_t*)delimiter);
}

uint32_t lexer_maps_get_operator(const char* word, const uint32_t word_size)
{
    VENOM_ASSERT(g_operators_map != NULL, "Operators map has not been initialized");

    void* operator = hashmap_get(g_operators_map, (const void*)word, word_size, NULL);

    if(operator == NULL)
    {
        return PyOperator_Unknown;
    }

    return *((uint32_t*)operator);
}

void lexer_maps_release()
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

void lexer_token_debug(PyToken* token)
{
    printf("%s: %.*s\n", 
           token_kind_to_string(token->kind),
           (int)token->length,
           token->start);
}

bool lexer_lex(char* buffer, Vector* tokens) 
{
    char* s = buffer;

    uint32_t indent_size = INDENT_SIZE_UNDEFINED;
    uint32_t indent_level = 0;
    uint32_t position = 1;
    uint32_t line = 1;

    while(*s != '\0') 
    {
        uint32_t string_literal_prefix = is_string_literal_start(s);

        if(string_literal_prefix != 0) 
        {
            s += string_literal_prefix;
            position += string_literal_prefix;

            char* start = s;
            uint32_t length = consume_string_literal(s, &position, &line);

            s += length;

            PyToken token = { 
                start, 
                length,
                PyTokenKind_Literal,
                PyLiteral_String,
                position,
                line
            };

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
                PyToken token = {
                    start, 
                    length,
                    PyTokenKind_Literal,
                    PyLiteral_Float,
                    position,
                    line
                };

                vector_push_back(tokens, &token);

                s += length;
                position += length;
                continue;
            }

            length = consume_int_literal(s);

            if(length > 0)
            {
                PyToken token = {
                    start, 
                    length,
                    PyTokenKind_Literal,
                    PyLiteral_Integer,
                    position,
                    line
                };

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

            uint32_t value = lexer_maps_get_keyword(start, length);

            if(value != PyKeyword_Unknown) 
            {
                PyToken token = {
                    start,
                    length,
                    PyTokenKind_Keyword,
                    value,
                    position - length,
                    line
                };

                vector_push_back(tokens, &token);
                continue;
            }

            value = lexer_maps_get_operator(start, length);

            if(value != PyOperator_Unknown)
            {
                PyToken token = {
                    start,
                    length,
                    PyTokenKind_Operator,
                    value,
                    position - length,
                    line
                };

                vector_push_back(tokens, &token);
                continue;
            }

            PyToken token = {
                start,
                length,
                PyTokenKind_Identifier,
                0,
                position - length,
                line
            };

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

            uint32_t value = lexer_maps_get_delimiter(start, length);

            if(value == PyDelimiter_Unknown) 
            {
                s -= length;
                position -= length;
                goto label_is_operator;
            }

            PyToken token = {
                start,
                length,
                PyTokenKind_Delimiter,
                value,
                position,
                line
            };

            vector_push_back(tokens, &token);
        }
        else if(is_operator_start(*s)) 
        {
label_is_operator: ;
            char* start = s;
            uint32_t length = is_operator(s);

            if(length > 0) 
            {
                position += length;

                uint32_t value = lexer_maps_get_operator(start, length);

                if(value == PyOperator_Unknown || length == PY_LEX_ERROR) 
                {
                    logger_log_error("Syntax Error: Invalid operator: %.*s", (int)length, start);
                    logger_log_error("Line %u, Position %u", line, position);
                    return false;
                }

                PyToken token = {
                    start,
                    length,
                    PyTokenKind_Operator,
                    value,
                    position,
                    line
                };

                vector_push_back(tokens, &token);

                s += length;
                continue;
            }
        }
        else if(*s == '\n') 
        {
            PyToken token = {
                NULL, 
                0,
                PyTokenKind_Newline,
                0,
                position,
                line
            };

            vector_push_back(tokens, &token);

            s++;

            line++;
            position = 0;

            while(*s != '\0' && *s == '\n') 
            {
                PyToken newline_token = {
                    NULL, 
                    0,
                    PyTokenKind_Newline,
                    0,
                    position,
                    line
                };

                vector_push_back(tokens, &newline_token);

                s++;
                line++;
            }

            uint32_t line_indent = 0;

            while(*s != '\0' && *s == ' ') 
            {
                line_indent++;
                s++;
                position++;
            }

            if(indent_size == INDENT_SIZE_UNDEFINED || indent_size == 0) 
            {
                indent_size = line_indent;
            }

            if(line_indent > indent_level) 
            {
                PyToken indent_token = {
                    NULL,
                    0,
                    PyTokenKind_Indent,
                    line_indent,
                    position,
                    line
                };

                vector_push_back(tokens, &indent_token);
            }
            else if(line_indent < indent_level) 
            {
                PyToken dedent_token = {
                    NULL, 
                    0,
                    PyTokenKind_Dedent,
                    line_indent,
                    position,
                    line
                };

                vector_push_back(tokens, &dedent_token);
            }

            indent_level = line_indent;
        }
        else
        {
            s++;
            position++;
        }
    }

    return true;
}