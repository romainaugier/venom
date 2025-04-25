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

typedef enum
{
     VTokenKind_Identifier,
     VTokenKind_Keyword,
     VTokenKind_Literal,
     VTokenKind_Operator,
     VTokenKind_Delimiter,
     VTokenKind_Newline,
     VTokenKind_Indent,
     VTokenKind_Dedent,
     VTokenKind_Unknown,
     VTokenKind_EOF,
} VTokenKind;

VENOM_API const char* token_kind_to_string(uint32_t token_type);

typedef enum
{
     VKeyword_False,
     VKeyword_Await,
     VKeyword_Else,
     VKeyword_Import,
     VKeyword_Pass,
     VKeyword_None,
     VKeyword_Break,
     VKeyword_Except,
     VKeyword_In,
     VKeyword_Raise,
     VKeyword_True,
     VKeyword_Class,
     VKeyword_Finally,
     VKeyword_Is,
     VKeyword_Return,
     VKeyword_And,
     VKeyword_Continue,
     VKeyword_For,
     VKeyword_Lambda,
     VKeyword_Try,
     VKeyword_As,
     VKeyword_Def,
     VKeyword_From,
     VKeyword_Nonlocal,
     VKeyword_While,
     VKeyword_Assert,
     VKeyword_Del,
     VKeyword_Global,
     VKeyword_Not,
     VKeyword_With,
     VKeyword_Async,
     VKeyword_Elif,
     VKeyword_If,
     VKeyword_Or,
     VKeyword_Yield,
     VKeyword_Unknown,
} VKeyword;

VENOM_API const char* v_keyword_to_string(VKeyword keyword);

typedef enum
{
     VDelimiter_LParen,
     VDelimiter_RParen,
     VDelimiter_LBracket,
     VDelimiter_RBracket,
     VDelimiter_LBrace,
     VDelimiter_RBrace,
     VDelimiter_Comma,
     VDelimiter_Colon,
     VDelimiter_Dot,
     VDelimiter_SemiColon,
     VDelimiter_At,
     VDelimiter_RightArrow,
     VDelimiter_Unknown,
} VDelimiter;

typedef enum
{
     VOperator_Addition,
     VOperator_Subtraction,
     VOperator_Multiplication,
     VOperator_Division,
     VOperator_Modulus,
     VOperator_Exponentiation,
     VOperator_FloorDivision,
     VOperator_Assign,
     VOperator_AdditionAssign,
     VOperator_SubtractionAssign,
     VOperator_MultiplicationAssign,
     VOperator_DivisionAssign,
     VOperator_ModulusAssign,
     VOperator_FloorDivisionAssign,
     VOperator_ExponentiationAssign,
     VOperator_BitwiseAndAssign,
     VOperator_BitwiseOrAssign,
     VOperator_BitwiseXorAssign,
     VOperator_BitwiseLShiftAssign,
     VOperator_BitwiseRShiftAssign,
     VOperator_BitwiseAnd,
     VOperator_BitwiseOr,
     VOperator_BitwiseXor,
     VOperator_BitwiseNot,
     VOperator_BitwiseLShift,
     VOperator_BitwiseRShift,
     VOperator_ComparatorEquals,
     VOperator_ComparatorNotEquals,
     VOperator_ComparatorGreaterThan,
     VOperator_ComparatorLessThan,
     VOperator_ComparatorGreaterEqualsThan,
     VOperator_ComparatorLessEqualsThan,
     VOperator_LogicalAnd,
     VOperator_LogicalOr,
     VOperator_LogicalNot,
     VOperator_IdentityIs,
     VOperator_IdentityIsNot,
     VOperator_MembershipIn,
     VOperator_MembershipNotIn,
     VOperator_Unknown,
} VOperator;

VENOM_API const char* v_operator_to_string(VOperator operator);

typedef enum
{
     VLiteral_String = 1,
     VLiteral_UnicodeString = 2,
     VLiteral_RawString = 3,
     VLiteral_FormattedString = 4,
     VLiteral_Bytes = 5,
     VLiteral_Integer = 6,
     VLiteral_Float = 7
} VLiteral;

VENOM_API void v_lexer_maps_init();

VENOM_API uint32_t v_lexer_maps_get_keyword(const char* word, const uint32_t word_size);

VENOM_API uint32_t v_lexer_maps_get_delimiter(const char* word, const uint32_t word_size);

VENOM_API uint32_t v_lexer_maps_get_operator(const char* word, const uint32_t word_size);

VENOM_API void v_lexer_maps_release();

typedef struct
{
     char* start;
     size_t length;
     uint32_t kind;
     uint32_t type;
     uint32_t position;
     uint32_t line;
} VToken;

VENOM_API void v_lexer_token_debug(VToken* token);

VENOM_API bool v_lexer_lex(char* buffer, Vector* tokens);

VENOM_CPP_END

#endif /* !defined(__VENOM_LEXER) */