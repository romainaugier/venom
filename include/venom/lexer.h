/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#pragma once

#if !defined(__VENOM_LEXER)
#define __VENOM_LEXER

#include "venom/venom.h"

#include "libromano/vector.h"

VENOM_CPP_ENTER

/*
    Help:
     - https://docs.python.org/3.8/reference/lexical_analysis.html 
     - https://docs.python.org/3/library/ast.html
*/

enum PyTokenKind {
    PyTokenKind_Identifier,
    PyTokenKind_Keyword,
    PyTokenKind_Literal,
    PyTokenKind_Operator,
    PyTokenKind_Delimiter,
    PyTokenKind_Newline,
    PyTokenKind_Indent,
    PyTokenKind_Dedent,
    PyTokenKind_Unknown,
};

VENOM_API const char* token_kind_to_string(uint32_t token_type);

enum PyKeyword {
    PyKeyword_False,
    PyKeyword_Await,
    PyKeyword_Else,
    PyKeyword_Import,
    PyKeyword_Pass,
    PyKeyword_None,
    PyKeyword_Break,
    PyKeyword_Except,
    PyKeyword_In,
    PyKeyword_Raise,
    PyKeyword_True,
    PyKeyword_Class,
    PyKeyword_Finally,
    PyKeyword_Is,
    PyKeyword_Return,
    PyKeyword_And,
    PyKeyword_Continue,
    PyKeyword_For,
    PyKeyword_Lambda,
    PyKeyword_Try,
    PyKeyword_As,
    PyKeyword_Def,
    PyKeyword_From,
    PyKeyword_Nonlocal,
    PyKeyword_While,
    PyKeyword_Assert,
    PyKeyword_Del,
    PyKeyword_Global,
    PyKeyword_Not,
    PyKeyword_With,
    PyKeyword_Async,
    PyKeyword_Elif,
    PyKeyword_If,
    PyKeyword_Or,
    PyKeyword_Yield,
    PyKeyword_Unknown,
};

enum PyDelimiter {
    PyDelimiter_LParen,
    PyDelimiter_RParen,
    PyDelimiter_LBracket,
    PyDelimiter_RBracket,
    PyDelimiter_LBrace,
    PyDelimiter_RBrace,
    PyDelimiter_Comma,
    PyDelimiter_Colon,
    PyDelimiter_Dot,
    PyDelimiter_SemiColon,
    PyDelimiter_At,
    PyDelimiter_RightArrow,
    PyDelimiter_Unknown,
};

enum PyOperator {
    PyOperator_Addition,
    PyOperator_Subtraction,
    PyOperator_Multiplication,
    PyOperator_Division,
    PyOperator_Modulus,
    PyOperator_Exponentiation,
    PyOperator_Floor_division,
    PyOperator_Assign,
    PyOperator_AdditionAssign,
    PyOperator_SubtractionAssign,
    PyOperator_MultiplicationAssign,
    PyOperator_DivisionAssign,
    PyOperator_ModulusAssign,
    PyOperator_FloorDivisionAssign,
    PyOperator_ExponentiationAssign,
    PyOperator_BitwiseAndAssign,
    PyOperator_BitwiseOrAssign,
    PyOperator_BitwiseXorAssign,
    PyOperator_BitwiseLShiftAssign,
    PyOperator_BitwiseRShiftAssign,
    PyOperator_BitwiseAnd,
    PyOperator_BitwiseOr,
    PyOperator_BitwiseXor,
    PyOperator_BitwiseNot,
    PyOperator_BitwiseLShift,
    PyOperator_BitwiseRShift,
    PyOperator_ComparatorEquals,
    PyOperator_ComparatorNotEquals,
    PyOperator_ComparatorGreaterThan,
    PyOperator_ComparatorLessThan,
    PyOperator_ComparatorGreaterEqualsThan,
    PyOperator_ComparatorLessEqualsThan,
    PyOperator_LogicalAnd,
    PyOperator_LogicalOr,
    PyOperator_LogicalNot,
    PyOperator_IdentityIs,
    PyOperator_IdentityIsNot,
    PyOperator_MembershipIn,
    PyOperator_MembershipNotIn,
    PyOperator_Unknown,
};

enum PyLiteral {
    PyLiteral_String = 1,
    PyLiteral_UnicodeString = 2,
    PyLiteral_RawString = 3,
    PyLiteral_FormattedString = 4,
    PyLiteral_Bytes = 5,
    PyLiteral_Integer = 6,
    PyLiteral_Float = 7
};

VENOM_API void lexer_maps_init();

VENOM_API uint32_t lexer_maps_get_keyword(const char* word, const uint32_t word_size);

VENOM_API uint32_t lexer_maps_get_delimiter(const char* word, const uint32_t word_size);

VENOM_API uint32_t lexer_maps_get_operator(const char* word, const uint32_t word_size);

VENOM_API void lexer_maps_release();

typedef struct {
    char* start;
    size_t length;
    uint32_t kind;
    uint32_t type;
    uint32_t position;
    uint32_t line;
} PyToken;

VENOM_API void lexer_token_debug(PyToken* token);

VENOM_API bool lexer_lex(char* buffer, Vector* tokens);

VENOM_CPP_END

#endif /* !defined(__VENOM_LEXER) */