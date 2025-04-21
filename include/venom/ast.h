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
    VASTNodeType_VASTSource,
    VASTNodeType_VASTClass,
    VASTNodeType_VASTFunction,
    VASTNodeType_VASTBody,
    VASTNodeType_VASTFor,
    VASTNodeType_VASTIf,
    VASTNodeType_VASTReturn,
    VASTNodeType_VASTAssignment,
    VASTNodeType_VASTUnOp,
    VASTNodeType_VASTBinOp,
    VASTNodeType_VASTTernOp,
    VASTNodeType_VASTDecorator,
    VASTNodeType_VASTAttribute,
    VASTNodeType_VASTVariable,
    VASTNodeType_VASTLiteral,
    VASTNodeType_VASTFCall,
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

struct VASTNode {
    VASTNodeType type;
};

struct VASTSource {
    VASTNode base;
    Vector* decls;
};

struct VASTClass {
    VASTNode base;
    String name;
    Vector* bases;
    Vector* attributes;
    Vector* functions;
    Vector* decorators;
};

struct VASTFunction {
    VASTNode base;
    String name;
    Vector* params;
    VASTBody* body;
    VType return_type;
    Vector* decorators;
};

struct VASTBody {
    VASTNode base;
    Vector* stmts;
};

struct VASTFor {
    VASTNode base;
    bool is_while;
    VASTNode* target;
    VASTNode* cond;
    VASTBody* body;
};

struct VASTIf {
    VASTNode base;
    VASTNode* condition;
    VASTBody* body;
    VASTNode* else_node;
};

struct VASTReturn {
    VASTNode base;
    VASTNode* value;
};

struct VASTAssignment {
    VASTNode base;
    VASTNode* target;
    VASTNode* value;
    VOperator op;
    VType type;
};

struct VASTUnOp {
    VASTNode base;
    VOperator op;
    VASTNode* operand;
};

struct VASTBinOp {
    VASTNode base;
    VOperator op;
    VASTNode* left;
    VASTNode* right;
};

struct VASTTernOp {
    VASTNode base;
    VASTNode* condition;
    VASTNode* if_expr;
    VASTNode* else_expr;
};

struct VASTDecorator {
    VASTNode base;
    String name;
};

struct VASTAttribute {
    VASTNode base;
    String name;
    VType type;
};

struct VASTVariable {
    VASTNode base;
    String name;
    VType type;
};

struct VASTLiteral {
    VASTNode base;
    VType lit_type;

    union {
        int64_t int_val;
        double float_val;
        String str_val;
        bool bool_val;

        struct {
            Vector* elements;
        } list_val;

        struct {
            Vector* keys;
            Vector* values;
        } dict_val;

        struct {
            Vector* elements;
        } tuple_val;

        struct {
            Vector* elements;
        } set_val;
    } value;
};

struct VASTFCall {
    VASTNode base;
    String function_name;
    Vector* args;

    struct {
        Vector* names;
        Vector* values;
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
VENOM_API VASTNode* v_ast_new_source(VAST* ast, Vector* decls);

VENOM_API VASTNode* v_ast_new_class(VAST* ast,
                                    const String name,
                                    Vector* bases,
                                    Vector* attributes,
                                    Vector* functions);

VENOM_API VASTNode* v_ast_new_function(VAST* ast,
                                       const String name,
                                       Vector* params,
                                       VType return_type,
                                       VASTBody* body);

VENOM_API VASTNode* v_ast_new_body(VAST* ast, 
                                   Vector* stmts);

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
                                           Vector* elements);

VENOM_API VASTNode* v_ast_new_literal_dict(VAST* ast,
                                           Vector* keys,
                                           Vector* values);

VENOM_API VASTNode* v_ast_new_literal_tuple(VAST* ast,
                                            Vector* elements);

VENOM_API VASTNode* v_ast_new_literal_set(VAST* ast,
                                          Vector* elements);

VENOM_API VASTNode* v_ast_new_fcall(VAST* ast,
                                    const String name,
                                    Vector* args,
                                    Vector* kwarg_names,
                                    Vector* kwarg_values);

#endif /* __VENOM_AST */
