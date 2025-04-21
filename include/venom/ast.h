/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#pragma once

#if !defined(__VENOM_AST)
#define __VENOM_AST

#include "venom/type.h"
#include "venom/lexer.h"

#include "libromano/arena.h"
#include "libromano/string.h"

VENOM_CPP_ENTER

typedef enum {
    VASTNodeType_VASTSource, /* Source file */
    VASTNodeType_VASTClass, /* Class definition */
    VASTNodeType_VASTFunction, /* Function definition */
    VASTNodeType_VASTBody, /* Scoped block of code inside a function, a source, a for/if */
    VASTNodeType_VASTFor, /* For/While loop */
    VASTNodeType_VASTIf, /* If/Else block */
    VASTNodeType_VASTReturn, /* Return stmt */
    VASTNodeType_VASTAssignment, /* Assignment, can be augmented/typed */
    VASTNodeType_VASTUnOp, /* Unary op */
    VASTNodeType_VASTBinOp, /* Binary op */
    VASTNodeType_VASTTernOp, /* Ternary op */
    VASTNodeType_VASTDecorator, /* Class/Function decorator */
    VASTNodeType_VASTAttribute, /* Class attribute */
    VASTNodeType_VASTVariable, /* Variable */
    VASTNodeType_VASTLiteral, /* Literal */
    VASTNodeType_VASTFCall, /* Function call */
} VASTNodeType;

typedef struct VASTNode VASTNode;
typedef struct VASTSource VASTSource;
typedef struct VASTClass VASTClass;
typedef struct VASTFunction VASTFunction;
typedef struct VASTBody VASTBody;
typedef struct VASTFor VASTFor;
typedef struct VASTIf VASTIf;
typedef struct VASTReturn VASTReturn;
typedef struct VASTAssignment VASTAssignment;
typedef struct VASTUnOp VASTUnOp;
typedef struct VASTBinOp VASTBinOp;
typedef struct VASTTernOp VASTTernOp;
typedef struct VASTDecorator VASTDecorator;
typedef struct VASTAttribute VASTAttribute;
typedef struct VASTVariable VASTVariable;
typedef struct VASTLiteral VASTLiteral;
typedef struct VASTFCall VASTFCall;

/* Base AST node structure */
struct VASTNode {
    VASTNodeType type;
};

/* Source file containing declarations */
struct VASTSource {
    VASTNode base;
    VASTNode** decls;
    uint32_t num_decls;
};

/* Class definition */
struct VASTClass {
    VASTNode base;
    String name;
    VASTNode** bases;      /* Base classes */
    uint32_t num_bases;
    VASTNode** attributes; /* Class attributes */
    uint32_t num_attributes;
    VASTNode** functions;  /* Class functions (or methods) */
    uint32_t num_functions;
};

/* Function/method definition */
struct VASTFunction {
    VASTNode base;
    String name;
    VASTNode** params;     /* Function parameters */
    uint32_t num_params;
    VASTBody* body;        /* Function body */
    VType return_type;
};

/* Code block */
struct VASTBody {
    VASTNode base;
    VASTNode** stmts;
    uint32_t num_stmts;
};

/* For loop structure */
struct VASTFor {
    VASTNode base;
    bool is_while;
    VASTNode* target; /* Iterable for for loop */
    VASTNode* cond;   /* Condition for while loop */
    VASTBody* body;
};

/* If-elif-else structure */
struct VASTIf {
    VASTNode base;
    VASTNode* condition;
    VASTBody* body;
    VASTNode* else_node;   /* Either another VASTIf (for elif) or VASTBody (for else) */
};

/* Return statement */
struct VASTReturn {
    VASTNode base;
    VASTNode* value;
};

/* Assignment statement, including augmented (+=, -=, etc.) */
struct VASTAssignment {
    VASTNode base;
    VASTNode* target;
    VASTNode* value;
    VOperator op;
    VType type;
};

/* Unary operation (-, not, ~) */
struct VASTUnOp {
    VASTNode base;
    VOperator op;
    VASTNode* operand;
};

/* Binary operation (+, -, *, /, etc.) */
struct VASTBinOp {
    VASTNode base;
    VOperator op;
    VASTNode* left;
    VASTNode* right;
};

/* Ternary operation (x if cond else y) */
struct VASTTernOp {
    VASTNode base;
    VASTNode* condition;
    VASTNode* if_expr;
    VASTNode* else_expr;
};

/* Class/Function decorator */
struct VASTDecorator {
    VASTNode base;
    String name;
};

/* Class attribute */
struct VASTAttribute {
    VASTNode base;
    String name;
    VType type;
};

/* Variable reference */
struct VASTVariable {
    VASTNode base;
    String name;
    VType type;
};

/* Literal value */
struct VASTLiteral {
    VASTNode base;
    VType lit_type;

    union {
        int64_t int_val;
        double float_val;
        String str_val;
        bool bool_val;

        struct {
            VASTNode** elements;
            uint32_t num_elements;
        } list_val;

        struct {
            VASTNode** keys;
            VASTNode** values;
            uint32_t num_pairs;
        } dict_val;

        struct {
            VASTNode** elements;
            uint32_t num_elements;
        } tuple_val;

        struct {
            VASTNode** elements;
            uint32_t num_elements;
        } set_val;
    } value;
};

/* Function call */
struct VASTFCall {
    VASTNode base;
    String function_name;
    VASTNode** args;
    uint32_t num_args;

    struct {
        String* names;
        VASTNode** values;
        uint32_t num_kwargs;
    } kwargs;
};

#define VAST_CAST(__type__, __node__) \
    ((__type__*)((__node__)->type == VASTNodeType_##__type__ ? (__node__) : NULL))

typedef struct {
    Arena data;
    VASTNode* root;
    String error;
} VAST;

/* VAST Functions */
VENOM_API VAST* v_ast_new();
VENOM_API void v_ast_debug(VAST* ast);
VENOM_API bool v_ast_from_tokens(VAST* ast, Vector* tokens);
VENOM_API void v_ast_destroy(VAST* ast);

/* VASTNodes functions */
VENOM_API VASTNode* v_ast_new_source(VAST* ast, 
                                     VASTNode** decls,
                                     const uint32_t num_decls);

VENOM_API VASTNode* v_ast_new_class(VAST* ast,
                                    const String name,
                                    VASTNode** bases,
                                    const uint32_t num_bases,
                                    VASTNode** attributes,
                                    const uint32_t num_attributes,
                                    VASTNode** functions,
                                    const uint32_t num_functions);

VENOM_API VASTNode* v_ast_new_function(VAST* ast,
                                       const String name,
                                       VASTNode** params,
                                       const uint32_t num_params,
                                       VType return_type,
                                       VASTBody* body);

VENOM_API VASTNode* v_ast_new_body(VAST* ast,
                                   VASTNode** stmts,
                                   const uint32_t num_stmts);

VENOM_API VASTNode* v_ast_new_for(VAST* ast,
                                  bool is_while,
                                  VASTNode* target,
                                  VASTNode* cond,
                                  VASTBody* body);

VENOM_API VASTNode* v_ast_new_if(VAST* ast,
                                 VASTNode* condition,
                                 VASTBody* body,
                                 VASTNode* else_node);

VENOM_API VASTNode* v_ast_new_return(VAST* ast,
                                     VASTNode* value);

VENOM_API VASTNode* v_ast_new_assignment(VAST* ast,
                                         VASTNode* target,
                                         VASTNode* value,
                                         const VOperator op,
                                         const VType type);

VENOM_API VASTNode* v_ast_new_unop(VAST* ast,
                                   const VOperator op, 
                                   VASTNode* operand);

VENOM_API VASTNode* v_ast_new_binop(VAST* ast, 
                                    const VOperator op,
                                    VASTNode* left,
                                    VASTNode* right);

VENOM_API VASTNode* v_ast_new_ternop(VAST* ast,
                                     VASTNode* condition,
                                     VASTNode* if_expr,
                                     VASTNode* else_expr);

VENOM_API VASTNode* v_ast_new_decorator(VAST* ast,
                                        const String name);

VENOM_API VASTNode* v_ast_new_attribute(VAST* ast,
                                        const String name,
                                        const VType type);

VENOM_API VASTNode* v_ast_new_variable(VAST* ast, 
                                       const String name,
                                       const VType type);

VENOM_API VASTNode* v_ast_new_literal_int(VAST* ast,
                                          const int64_t value);

VENOM_API VASTNode* v_ast_new_literal_float(VAST* ast,
                                            const double value);

VENOM_API VASTNode* v_ast_new_literal_string(VAST* ast,
                                             const String value);

VENOM_API VASTNode* v_ast_new_literal_bool(VAST* ast,
                                           const bool value);

VENOM_API VASTNode* v_ast_new_literal_none(VAST* ast);

VENOM_API VASTNode* v_ast_new_literal_list(VAST* ast,
                                           VASTNode** elements,
                                           const uint32_t num_elements);

VENOM_API VASTNode* v_ast_new_literal_dict(VAST* ast,
                                           VASTNode** keys,
                                           VASTNode** values,
                                           const uint32_t num_pairs);

VENOM_API VASTNode* v_ast_new_literal_tuple(VAST* ast,
                                            VASTNode** elements,
                                            const uint32_t num_elements);

VENOM_API VASTNode* v_ast_new_literal_set(VAST* ast,
                                          VASTNode** elements,
                                          const uint32_t num_elements);

VENOM_API VASTNode* v_ast_new_fcall(VAST* ast,
                                    const String name, 
                                    VASTNode** args,
                                    const uint32_t num_args,
                                    String* kwarg_names,
                                    VASTNode** kwarg_values,
                                    const uint32_t num_kwargs);

VENOM_CPP_END

#endif /* !defined(__VENOM_AST) */