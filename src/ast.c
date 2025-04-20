/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/ast.h"

#include "libromano/memory.h"

/* VAST functions */

VAST* v_ast_new()
{
    VAST* ast = (VAST*)malloc(sizeof(VAST));

    arena_init(&ast->data, 1024);
    ast->root = NULL;
    ast->error = NULL;

    return ast;
}

void v_ast_debug(VAST* ast)
{
    if(ast == NULL)
    {
        return;
    }
}

void v_ast_destroy(VAST* ast)
{
    if(ast != NULL)
    {
        arena_destroy(&ast->data);
        
        if(ast->error != NULL)
        {
            string_free(ast->error);
        }
    }
}

/* VASTNodes functions */

VASTNode* v_ast_new_source(VAST* ast,
                           VASTNode** decls,
                           const uint32_t num_decls)
{
    VASTSource source = { VASTNodeType_VASTSource,
                          decls,
                          num_decls };

    void* addr = arena_push(&ast->data, &source, sizeof(VASTSource));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_class(VAST* ast,
                          const String name,
                          VASTNode** bases,
                          const uint32_t num_bases,
                          VASTNode** attributes,
                          const uint32_t num_attributes,
                          VASTNode** functions,
                          const uint32_t num_functions)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_function(VAST* ast,
                             const String name,
                             VASTNode** params,
                             const uint32_t num_params,
                             VASTNode* return_type,
                             VASTBody* body)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_body(VAST* ast,
                         VASTNode** stmts,
                         const uint32_t num_stmts)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_for(VAST* ast,
                        bool is_while,
                        VASTNode* target,
                        VASTNode* cond,
                        VASTBody* body)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_if(VAST* ast,
                       VASTNode* condition,
                       VASTBody* body,
                       VASTNode* else_node)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_return(VAST* ast,
                           VASTNode* value)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_assignment(VAST* ast,
                               VASTNode* target,
                               VASTNode* value,
                               const VOperator op,
                               const VType type)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_unop(VAST* ast,
                         const VOperator op, 
                         VASTNode* operand)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_binop(VAST* ast, 
                          const VOperator op,
                          VASTNode* left,
                          VASTNode* right)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_ternop(VAST* ast,
                           VASTNode* condition,
                           VASTNode* if_expr,
                           VASTNode* else_expr)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_attribute(VAST* ast,
                              const String name,
                              const VType type)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_variable(VAST* ast, 
                             const String name,
                             const VType type)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_int(VAST* ast,
                                const int64_t value)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_float(VAST* ast,
                                  const double value)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_string(VAST* ast,
                                   const String value)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_bool(VAST* ast,
                                 const bool value)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_none(VAST* ast)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_list(VAST* ast,
                                 VASTNode** elements,
                                 const uint32_t num_elements)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_dict(VAST* ast,
                                 VASTNode** keys,
                                 VASTNode** values,
                                 const uint32_t num_pairs)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_tuple(VAST* ast,
                                  VASTNode** elements,
                                  const uint32_t num_elements)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_literal_set(VAST* ast,
                                VASTNode** elements,
                                const uint32_t num_elements)
{
    VENOM_NOT_IMPLEMENTED;
}

VASTNode* v_ast_new_fcall(VAST* ast,
                          const String name, 
                          VASTNode** args,
                          const uint32_t num_args,
                          String* kwarg_names,
                          VASTNode** kwarg_values,
                          const uint32_t num_kwargs)
{
    VENOM_NOT_IMPLEMENTED;
}