/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/ast.h"

#include "libromano/memory.h"

/*******************************/
/* VAST functions */

VAST* v_ast_new()
{
    VAST* ast = (VAST*)malloc(sizeof(VAST));

    arena_init(&ast->data, 1024);
    ast->root = NULL;
    ast->error = NULL;

    return ast;
}

/* VAST Debugging */

#define VASSERT_TYPE(__type__, __node__) \
            VENOM_ASSERT(__node__ != NULL, "Wrong type casting, should have been " VENOM_STRIFY(__type__));

VENOM_FORCE_INLINE void print_indent(int level) 
{
    for(int i = 0; i < level; i++) 
    {
        printf("  ");
    }
}

void v_ast_node_debug(VASTNode* node, int indent_level) 
{
    if(node == NULL) 
    {
        print_indent(indent_level);
        printf("<NULL>\n");
        return;
    }

    print_indent(indent_level);
    
    switch(node->type) 
    {
        case VASTNodeType_VASTSource: 
        {
            VASTSource* source = VAST_CAST(VASTSource, node);
            VASSERT_TYPE(VASTSource, source);

            printf("Source (%u declarations)\n", source->num_decls);

            for(uint32_t i = 0; i < source->num_decls; i++) 
            {
                v_ast_node_debug(source->decls[i], indent_level + 1);
            }

            break;
        }
        
        case VASTNodeType_VASTClass: 
        {
            VASTClass* class = VAST_CAST(VASTClass, node);
            VASSERT_TYPE(VASTClass, class);

            printf("Class \"%s\" (%u bases, %u attributes, %u functions)\n", 
                   class->name,
                   class->num_bases, 
                   class->num_attributes,
                   class->num_functions);
            
            if(class->num_bases > 0) 
            {
                print_indent(indent_level + 1);
                printf("Bases:\n");

                for(uint32_t i = 0; i < class->num_bases; i++) 
                {
                    v_ast_node_debug(class->bases[i], indent_level + 2);
                }
            }
            
            if(class->num_attributes > 0) 
            {
                print_indent(indent_level + 1);
                printf("Attributes:\n");

                for(uint32_t i = 0; i < class->num_attributes; i++) 
                {
                    v_ast_node_debug(class->attributes[i], indent_level + 2);
                }
            }
            
            if(class->num_functions > 0)
            {
                print_indent(indent_level + 1);
                printf("Functions:\n");

                for(uint32_t i = 0; i < class->num_functions; i++) 
                {
                    v_ast_node_debug(class->functions[i], indent_level + 2);
                }
            }

            break;
        }
        
        case VASTNodeType_VASTFunction: 
        {
            VASTFunction* func = VAST_CAST(VASTFunction, node);
            VASSERT_TYPE(VASTFunction*, func);

            printf("Function \"%s\" (%u parameters, return_type: %s)\n", 
                   func->name,
                   func->num_params, 
                   v_type_to_string(func->return_type));
            
            if(func->num_params > 0) 
            {
                print_indent(indent_level + 1);
                printf("Parameters:\n");

                for(uint32_t i = 0; i < func->num_params; i++) 
                {
                    v_ast_node_debug(func->params[i], indent_level + 2);
                }
            }
            
            if(func->body) 
            {
                print_indent(indent_level + 1);
                printf("Body:\n");
                v_ast_node_debug((VASTNode*)func->body, indent_level + 2);
            }

            break;
        }
        
        case VASTNodeType_VASTBody: 
        {
            VASTBody* body = VAST_CAST(VASTBody, node);
            VASSERT_TYPE(VASTBody*, body);

            printf("Body (%u statements)\n", body->num_stmts);

            for(uint32_t i = 0; i < body->num_stmts; i++) 
            {
                v_ast_node_debug(body->stmts[i], indent_level + 1);
            }

            break;
        }
        
        case VASTNodeType_VASTFor: 
        {
            VASTFor* forloop = VAST_CAST(VASTFor, node);
            VASSERT_TYPE(VASTFor*, forloop);

            if(forloop->is_while) 
            {
                printf("While loop\n");
                print_indent(indent_level + 1);
                printf("Condition:\n");
                v_ast_node_debug(forloop->cond, indent_level + 2);
            } 
            else 
            {
                printf("For loop\n");
                print_indent(indent_level + 1);
                printf("Target:\n");
                v_ast_node_debug(forloop->target, indent_level + 2);
            }
            
            print_indent(indent_level + 1);
            printf("Body:\n");
            v_ast_node_debug((VASTNode*)forloop->body, indent_level + 2);

            break;
        }
        
        case VASTNodeType_VASTIf: 
        {
            VASTIf* if_ = VAST_CAST(VASTIf, node);
            VASSERT_TYPE(VASTIf*, if_);

            printf("If statement\n");
            
            print_indent(indent_level + 1);
            printf("Condition:\n");
            v_ast_node_debug(if_->condition, indent_level + 2);
            
            print_indent(indent_level + 1);
            printf("Body:\n");
            v_ast_node_debug((VASTNode*)if_->body, indent_level + 2);
            
            if(if_->else_node) 
            {
                print_indent(indent_level + 1);
                printf("Else/Elif:\n");
                v_ast_node_debug(if_->else_node, indent_level + 2);
            }

            break;
        }
        
        case VASTNodeType_VASTReturn: 
        {
            VASTReturn* ret = VAST_CAST(VASTReturn, node);
            VASSERT_TYPE(VASTReturn*, ret);

            printf("Return\n");

            if(ret->value) 
            {
                print_indent(indent_level + 1);
                printf("Value:\n");
                v_ast_node_debug(ret->value, indent_level + 2);
            }

            break;
        }
        
        case VASTNodeType_VASTAssignment: 
        {
            VASTAssignment* assign = VAST_CAST(VASTAssignment, node);
            VASSERT_TYPE(VASTAssignment*, assign);

            printf("Assignment (op: %d, type: %s)\n", assign->op, v_type_to_string(assign->type));
            
            print_indent(indent_level + 1);
            printf("Target:\n");
            v_ast_node_debug(assign->target, indent_level + 2);
            
            print_indent(indent_level + 1);
            printf("Value:\n");
            v_ast_node_debug(assign->value, indent_level + 2);

            break;
        }
        
        case VASTNodeType_VASTUnOp: 
        {
            VASTUnOp* unop = VAST_CAST(VASTUnOp, node);
            VASSERT_TYPE(VASTUnOp*, unop);

            printf("Unary Operation (op: %d)\n", unop->op);
            
            print_indent(indent_level + 1);
            printf("Operand:\n");
            v_ast_node_debug(unop->operand, indent_level + 2);

            break;
        }
        
        case VASTNodeType_VASTBinOp: 
        {
            VASTBinOp* binop = VAST_CAST(VASTBinOp, node);
            VASSERT_TYPE(VASTBinOp*, binop);

            printf("Binary Operation (op: %d)\n", binop->op);
            
            print_indent(indent_level + 1);
            printf("Left:\n");
            v_ast_node_debug(binop->left, indent_level + 2);
            
            print_indent(indent_level + 1);
            printf("Right:\n");
            v_ast_node_debug(binop->right, indent_level + 2);

            break;
        }
        
        case VASTNodeType_VASTTernOp: 
        {
            VASTTernOp* ternop = VAST_CAST(VASTTernOp, node);
            VASSERT_TYPE(VASTTernOp*, ternop);

            printf("Ternary Operation\n");
            
            print_indent(indent_level + 1);
            printf("Condition:\n");
            v_ast_node_debug(ternop->condition, indent_level + 2);
            
            print_indent(indent_level + 1);
            printf("If Expr:\n");
            v_ast_node_debug(ternop->if_expr, indent_level + 2);
            
            print_indent(indent_level + 1);
            printf("Else Expr:\n");
            v_ast_node_debug(ternop->else_expr, indent_level + 2);

            break;
        }
        
        case VASTNodeType_VASTDecorator: 
        {
            VASTDecorator* decorator = VAST_CAST(VASTDecorator, node);
            VASSERT_TYPE(VASTDecorator*, decorator);

            printf("Decorator \"%s\"\n", decorator->name);

            break;
        }
        
        case VASTNodeType_VASTAttribute: 
        {
            VASTAttribute* attr = VAST_CAST(VASTAttribute, node);
            VASSERT_TYPE(VASTAttribute*, attr);

            printf("Attribute \"%s\" (type: %s)\n", attr->name, v_type_to_string(attr->type));

            break;
        }
        
        case VASTNodeType_VASTVariable: 
        {
            VASTVariable* var = VAST_CAST(VASTVariable, node);
            VASSERT_TYPE(VASTVariable*, var);

            printf("Variable \"%s\" (type: %s)\n", var->name, v_type_to_string(var->type));

            break;
        }
        
        case VASTNodeType_VASTLiteral: 
        {
            VASTLiteral* lit = VAST_CAST(VASTLiteral, node);
            VASSERT_TYPE(VASTLiteral*, lit);

            printf("Literal (type: %s) ", v_type_to_string(lit->lit_type));
            
            switch(lit->lit_type) 
            {
                case VType_Int:
                    printf("Int: %li\n", lit->value.int_val);
                    break;
                    
                case VType_Float:
                    printf("Float: %f\n", lit->value.float_val);
                    break;
                    
                case VType_String:
                    printf("String: \"%s\"\n", lit->value.str_val);
                    break;
                    
                case VType_Bool:
                    printf("Bool: %s\n", lit->value.bool_val ? "true" : "false");
                    break;
                    
                case VType_None:
                    printf("None\n");
                    break;
                    
                case VType_List:
                    printf("List (%u elements)\n", lit->value.list_val.num_elements);

                    for(uint32_t i = 0; i < lit->value.list_val.num_elements; i++) 
                    {
                        v_ast_node_debug(lit->value.list_val.elements[i], indent_level + 1);
                    }

                    break;
                    
                case VType_Dict:
                    printf("Dict (%u pairs)\n", lit->value.dict_val.num_pairs);

                    for(uint32_t i = 0; i < lit->value.dict_val.num_pairs; i++) 
                    {
                        print_indent(indent_level + 1);
                        printf("Key %u:\n", i);
                        v_ast_node_debug(lit->value.dict_val.keys[i], indent_level + 2);
                        
                        print_indent(indent_level + 1);
                        printf("Value %u:\n", i);
                        v_ast_node_debug(lit->value.dict_val.values[i], indent_level + 2);
                    }

                    break;
                    
                case VType_Tuple:
                    printf("Tuple (%u elements)\n", lit->value.tuple_val.num_elements);

                    for(uint32_t i = 0; i < lit->value.tuple_val.num_elements; i++) 
                    {
                        v_ast_node_debug(lit->value.tuple_val.elements[i], indent_level + 1);
                    }

                    break;
                    
                case VType_Set:
                    printf("Set (%u elements)\n", lit->value.set_val.num_elements);

                    for(uint32_t i = 0; i < lit->value.set_val.num_elements; i++) 
                    {
                        v_ast_node_debug(lit->value.set_val.elements[i], indent_level + 1);
                    }

                    break;
                    
                default:
                    printf("Unknown literal type\n");
                    break;
            }

            break;
        }
        
        case VASTNodeType_VASTFCall: 
        {
            VASTFCall* fcall = VAST_CAST(VASTFCall, node);
            VASSERT_TYPE(VASTFCall*, fcall);

            printf("Function Call \"%s\" (%u args, %u kwargs)\n", 
                   fcall->function_name,
                   fcall->num_args, 
                   fcall->kwargs.num_kwargs);
            
            if(fcall->num_args > 0) 
            {
                print_indent(indent_level + 1);
                printf("Arguments:\n");

                for(uint32_t i = 0; i < fcall->num_args; i++) 
                {
                    v_ast_node_debug(fcall->args[i], indent_level + 2);
                }
            }
            
            if(fcall->kwargs.num_kwargs > 0) 
            {
                print_indent(indent_level + 1);
                printf("Keyword Arguments:\n");

                for(uint32_t i = 0; i < fcall->kwargs.num_kwargs; i++) 
                {
                    print_indent(indent_level + 2);
                    printf("\"%s\":\n", fcall->kwargs.names[i]);

                    v_ast_node_debug(fcall->kwargs.values[i], indent_level + 3);
                }
            }
            break;
        }
        
        default:
            printf("Unknown node type: %d\n", node->type);
            break;
    }
}

void v_ast_debug(VAST* ast) 
{
    if(ast == NULL || ast->root == NULL) 
    {
        printf("AST is empty or NULL\n");
        return;
    }
    
    v_ast_node_debug(ast->root, 0);
}

/* VAST Parsing */

bool v_ast_from_tokens(VAST* ast, Vector* tokens)
{
    return true;
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

        free(ast);

        ast = NULL;
    }
}

/*******************************/
/* VASTNodes functions */

VASTNode* v_ast_new_source(VAST* ast,
                           VASTNode** decls,
                           const uint32_t num_decls)
{
    VASTSource source = { 
        { VASTNodeType_VASTSource },
        decls,
        num_decls 
    };

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
    VASTClass class = { 
        { VASTNodeType_VASTClass },
        name,
        bases,
        num_bases,
        attributes,
        num_attributes,
        functions,
        num_functions
    };

    void* addr = arena_push(&ast->data, &class, sizeof(VASTClass));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_function(VAST* ast,
                             const String name,
                             VASTNode** params,
                             const uint32_t num_params,
                             VType return_type,
                             VASTBody* body)
{
    VASTFunction func = {
        { VASTNodeType_VASTFunction },
        name,
        params,
        num_params,
        body,
        return_type,
    };

    void* addr = arena_push(&ast->data, &func, sizeof(VASTFunction));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_body(VAST* ast,
                         VASTNode** stmts,
                         const uint32_t num_stmts)
{
    VASTBody body = {
        { VASTNodeType_VASTBody },
        stmts,
        num_stmts,
    };

    void* addr = arena_push(&ast->data, &body, sizeof(VASTBody));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_for(VAST* ast,
                        bool is_while,
                        VASTNode* target,
                        VASTNode* cond,
                        VASTBody* body)
{
    VASTFor for_loop = {
        { VASTNodeType_VASTFor },
        is_while,
        target,
        cond,
        body,
    };

    void* addr = arena_push(&ast->data, &for_loop, sizeof(VASTFor));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_if(VAST* ast,
                       VASTNode* condition,
                       VASTBody* body,
                       VASTNode* else_node)
{
    VASTIf if_ = {
        { VASTNodeType_VASTIf },
        condition,
        body,
        else_node,
    };

    void* addr = arena_push(&ast->data, &if_, sizeof(VASTIf));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_return(VAST* ast,
                           VASTNode* value)
{
    VASTReturn ret = {
        { VASTNodeType_VASTReturn },
        value,
    };

    void* addr = arena_push(&ast->data, &ret, sizeof(VASTReturn));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_assignment(VAST* ast,
                               VASTNode* target,
                               VASTNode* value,
                               const VOperator op,
                               const VType type)
{
    VASTAssignment assignment = {
        { VASTNodeType_VASTAssignment },
        target,
        value,
        op,
        type,
    };

    void* addr = arena_push(&ast->data, &assignment, sizeof(VASTAssignment));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_unop(VAST* ast,
                         const VOperator op, 
                         VASTNode* operand)
{
    VASTUnOp unop = {
        { VASTNodeType_VASTUnOp },
        op,
        operand,
    };

    void* addr = arena_push(&ast->data, &unop, sizeof(VASTUnOp));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_binop(VAST* ast, 
                          const VOperator op,
                          VASTNode* left,
                          VASTNode* right)
{
    VASTBinOp binop = {
        { VASTNodeType_VASTBinOp },
        op,
        left,
        right,
    };

    void* addr = arena_push(&ast->data, &binop, sizeof(VASTBinOp));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_ternop(VAST* ast,
                           VASTNode* condition,
                           VASTNode* if_expr,
                           VASTNode* else_expr)
{
    VASTTernOp ternop = {
        { VASTNodeType_VASTTernOp },
        condition,
        if_expr,
        else_expr,
    };

    void* addr = arena_push(&ast->data, &ternop, sizeof(VASTTernOp));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_decorator(VAST* ast,
                              const String name)
{
    VASTDecorator decorator = {
        { VASTNodeType_VASTDecorator },
        name,
    };

    void* addr = arena_push(&ast->data, &decorator, sizeof(VASTDecorator));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_attribute(VAST* ast,
                              const String name,
                              const VType type)
{
    VASTAttribute attribute = {
        { VASTNodeType_VASTAttribute },
        name,
        type,
    };

    void* addr = arena_push(&ast->data, &attribute, sizeof(VASTAttribute));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_variable(VAST* ast, 
                             const String name,
                             const VType type)
{
    VASTVariable var = {
        { VASTNodeType_VASTVariable },
        name,
        type,
    };

    void* addr = arena_push(&ast->data, &var, sizeof(VASTVariable));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_int(VAST* ast,
                                const int64_t value)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Int;
    lit.value.int_val = value;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_float(VAST* ast,
                                  const double value)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Float;
    lit.value.float_val = value;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_string(VAST* ast,
                                   const String value)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_String;
    lit.value.str_val = value;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_bool(VAST* ast,
                                 const bool value)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Bool;
    lit.value.bool_val = value;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_none(VAST* ast)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_None;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_list(VAST* ast,
                                 VASTNode** elements,
                                 const uint32_t num_elements)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_List;
    lit.value.list_val.elements = elements;
    lit.value.list_val.num_elements = num_elements;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_dict(VAST* ast,
                                 VASTNode** keys,
                                 VASTNode** values,
                                 const uint32_t num_pairs)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Dict;
    lit.value.dict_val.keys = keys;
    lit.value.dict_val.values = values;
    lit.value.dict_val.num_pairs = num_pairs;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_tuple(VAST* ast,
                                  VASTNode** elements,
                                  const uint32_t num_elements)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Tuple;
    lit.value.tuple_val.elements = elements;
    lit.value.tuple_val.num_elements = num_elements;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_set(VAST* ast,
                                VASTNode** elements,
                                const uint32_t num_elements)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Set;
    lit.value.set_val.elements = elements;
    lit.value.set_val.num_elements = num_elements;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_fcall(VAST* ast,
                          const String name, 
                          VASTNode** args,
                          const uint32_t num_args,
                          String* kwarg_names,
                          VASTNode** kwarg_values,
                          const uint32_t num_kwargs)
{
    VASTFCall fcall = {
        { VASTNodeType_VASTFCall },
        name,
        args,
        num_args,
        {
            kwarg_names,
            kwarg_values,
            num_kwargs,
        }
    };

    void* addr = arena_push(&ast->data, &fcall, sizeof(VASTFCall));

    return (VASTNode*)addr;
}