/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/ast.h"

#include "libromano/memory.h"

/*******************************/
/* VAST functions */

/* VAST Construction */

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

            printf("Source (%u declarations)\n", vector_size(source->decls));

            for(uint32_t i = 0; i < vector_size(source->decls); i++) 
            {
                v_ast_node_debug((VASTNode*)vector_at(source->decls, i), indent_level + 1);
            }

            break;
        }
        
        case VASTNodeType_VASTClass: 
        {
            VASTClass* class = VAST_CAST(VASTClass, node);
            VASSERT_TYPE(VASTClass, class);

            printf("Class \"%s\" (%u bases, %u attributes, %u functions)\n", 
                   class->name,
                   vector_size(class->bases), 
                   vector_size(class->attributes),
                   vector_size(class->functions));
            
            if(vector_size(class->bases) > 0) 
            {
                print_indent(indent_level + 1);
                printf("Bases:\n");

                for(uint32_t i = 0; i < vector_size(class->bases); i++) 
                {
                    v_ast_node_debug((VASTNode*)vector_at(class->bases, i), indent_level + 2);
                }
            }
            
            if(vector_size(class->attributes) > 0) 
            {
                print_indent(indent_level + 1);
                printf("Attributes:\n");

                for(uint32_t i = 0; i < vector_size(class->attributes); i++) 
                {
                    v_ast_node_debug((VASTNode*)vector_at(class->attributes, i), indent_level + 2);
                }
            }
            
            if(vector_size(class->functions) > 0)
            {
                print_indent(indent_level + 1);
                printf("Functions:\n");

                for(uint32_t i = 0; i < vector_size(class->functions); i++) 
                {
                    v_ast_node_debug((VASTNode*)vector_at(class->functions, i), indent_level + 2);
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
                   vector_size(func->params), 
                   v_type_to_string(func->return_type));
            
            if(vector_size(func->params) > 0) 
            {
                print_indent(indent_level + 1);
                printf("Parameters:\n");

                for(uint32_t i = 0; i < vector_size(func->params); i++) 
                {
                    v_ast_node_debug((VASTNode*)vector_at(func->params, i), indent_level + 2);
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

            printf("Body (%u statements)\n", vector_size(body->stmts));

            for(uint32_t i = 0; i < vector_size(body->stmts); i++) 
            {
                v_ast_node_debug((VASTNode*)vector_at(body->stmts, i), indent_level + 1);
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

            printf("Unary Operation (op: %s)\n", v_operator_to_string(unop->op));
            
            print_indent(indent_level + 1);
            printf("Operand:\n");
            v_ast_node_debug(unop->operand, indent_level + 2);

            break;
        }
        
        case VASTNodeType_VASTBinOp: 
        {
            VASTBinOp* binop = VAST_CAST(VASTBinOp, node);
            VASSERT_TYPE(VASTBinOp*, binop);

            printf("Binary Operation (op: %s)\n", v_operator_to_string(binop->op));
            
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
                    printf("List (%u elements)\n", vector_size(lit->value.list_val.elements));

                    for(uint32_t i = 0; i < vector_size(lit->value.list_val.elements); i++) 
                    {
                        v_ast_node_debug((VASTNode*)vector_at(lit->value.list_val.elements, i), indent_level + 1);
                    }

                    break;
                    
                case VType_Dict: 
                {
                    size_t num_pairs = vector_size(lit->value.dict_val.keys);
                    printf("Dict (%zu pairs)\n", num_pairs);

                    for(size_t i = 0; i < num_pairs; i++) 
                    {
                        print_indent(indent_level + 1);
                        printf("Key %zu:\n", i);
                        v_ast_node_debug((VASTNode*)vector_at(lit->value.dict_val.keys, i), indent_level + 2);

                        print_indent(indent_level + 1);
                        printf("Value %zu:\n", i);
                        v_ast_node_debug((VASTNode*)vector_at(lit->value.dict_val.values, i), indent_level + 2);
                    }

                    break;
                }

                case VType_Tuple:
                {
                    size_t num_elements = vector_size(lit->value.tuple_val.elements);
                    printf("Tuple (%zu elements)\n", num_elements);

                    for(size_t i = 0; i < num_elements; i++) 
                    {
                        v_ast_node_debug((VASTNode*)vector_at(lit->value.tuple_val.elements, i), indent_level + 1);
                    }

                    break;
                }

                case VType_Set: 
                {
                    size_t num_elements = vector_size(lit->value.set_val.elements);
                    printf("Set (%zu elements)\n", num_elements);

                    for(size_t i = 0; i < num_elements; i++) 
                    {
                        v_ast_node_debug((VASTNode*)vector_at(lit->value.set_val.elements, i), indent_level + 1);
                    }

                    break;
                }
                    
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

            size_t num_args = vector_size(fcall->args);
            size_t num_kwargs = vector_size(fcall->kwargs.names);

            printf("Function Call \"%s\" (%zu args, %zu kwargs)\n",
                fcall->function_name,
                num_args,
                num_kwargs);

            if(num_args > 0) 
            {
                print_indent(indent_level + 1);
                printf("Arguments:\n");

                for(size_t i = 0; i < num_args; i++) 
                {
                    v_ast_node_debug((VASTNode*)vector_at(fcall->args, i), indent_level + 2);
                }
            }

            if(num_kwargs > 0) 
            {
                print_indent(indent_level + 1);
                printf("Keyword Arguments:\n");

                for (size_t i = 0; i < num_kwargs; i++) 
                {
                    print_indent(indent_level + 2);
                    printf("\"%s\":\n", *(String*)vector_at(fcall->kwargs.names, i));
                    v_ast_node_debug((VASTNode*)vector_at(fcall->kwargs.values, i), indent_level + 3);
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

typedef struct {
    Vector* tokens;
    size_t current;
    VAST* ast;
    String error;
} VParser;

VENOM_FORCE_INLINE bool v_parser_is_at_end(VParser* parser)
{
    return parser->current >= vector_size(parser->tokens);
}

VENOM_FORCE_INLINE VToken* v_parser_peek(VParser* parser)
{
    if(v_parser_is_at_end(parser))
    {
        return NULL;
    }

    return (VToken*)vector_at(parser->tokens, parser->current);
}

VENOM_FORCE_INLINE VToken* v_parser_advance(VParser* parser)
{
    if(!v_parser_is_at_end(parser))
    {
        parser->current++;
    }

    return (VToken*)vector_at(parser->tokens, parser->current - 1);
}

VENOM_FORCE_INLINE bool v_parser_check(VParser* parser, VTokenKind kind) 
{
    if(v_parser_is_at_end(parser))
    {
        return false;
    }

    return v_parser_peek(parser)->kind == kind;
}

VENOM_FORCE_INLINE bool v_parser_check_keyword(VParser* parser, VKeyword keyword) 
{
    if(v_parser_is_at_end(parser))
    {
        return false;
    }

    VToken* token = v_parser_peek(parser);

    return token->kind == VTokenKind_Keyword && token->type == keyword;
}

VENOM_FORCE_INLINE bool v_parser_check_delimiter(VParser* parser, VDelimiter delimiter) 
{
    if(v_parser_is_at_end(parser)) 
    {
        return false;
    }

    VToken* token = v_parser_peek(parser);

    return token->kind == VTokenKind_Delimiter && token->type == delimiter;
}

VENOM_FORCE_INLINE bool v_parser_check_operator(VParser* parser, VOperator op) 
{
    if(v_parser_is_at_end(parser))
    {
        return false;
    }

    VToken* token = v_parser_peek(parser);

    return token->kind == VTokenKind_Operator && token->type == op;
}

VENOM_FORCE_INLINE bool v_parser_match(VParser* parser, VTokenKind kind) 
{
    if(v_parser_check(parser, kind)) 
    {
        v_parser_advance(parser);
        return true;
    }

    return false;
}

VENOM_FORCE_INLINE bool v_parser_match_keyword(VParser* parser, VKeyword keyword) 
{
    if(v_parser_check_keyword(parser, keyword)) 
    {
        v_parser_advance(parser);
        return true;
    }

    return false;
}

VENOM_FORCE_INLINE bool v_parser_match_delimiter(VParser* parser, VDelimiter delimiter) 
{
    if(v_parser_check_delimiter(parser, delimiter)) 
    {
        v_parser_advance(parser);
        return true;
    }

    return false;
}

VENOM_FORCE_INLINE bool v_parser_match_operator(VParser* parser, VOperator op) 
{
    if(v_parser_check_operator(parser, op)) 
    {
        v_parser_advance(parser);
        return true;
    }

    return false;
}

VENOM_FORCE_INLINE VToken* v_parser_consume(VParser* parser,
                                            VTokenKind kind,
                                            const char* error_message) 
{
    if(v_parser_check(parser, kind)) 
    {
        return v_parser_advance(parser);
    }

    v_parser_set_error(parser, error_message);
    
    return NULL;
}

VENOM_FORCE_INLINE VToken* v_parser_consume_delimiter(VParser* parser, 
                                                      VDelimiter delimiter,
                                                      const char* error_message) 
{
    if(v_parser_check_delimiter(parser, delimiter)) 
    {
        return v_parser_advance(parser);
    }

    v_parser_set_error(parser, error_message);

    return NULL;
}

VENOM_FORCE_INLINE VToken* v_parser_consume_keyword(VParser* parser,
                                                    VKeyword keyword,
                                                    const char* error_message) 
{
    if(v_parser_check_keyword(parser, keyword)) 
    {
        return v_parser_advance(parser);
    }

    v_parser_set_error(parser, error_message);

    return NULL;
}

void v_parser_set_error(VParser* parser, const char* message) 
{
    if(parser->error != NULL) 
    {
        return;
    }

    VToken* token = v_parser_peek(parser);
    const uint32_t line_info = token->line > 0 ? token->line : 0;

    parser->error = string_newf("Parsing error at line %d: %s",
                                line_info,
                                message);
}

VASTNode* v_parse_source(VParser* parser);
VASTNode* v_parse_declaration(VParser* parser);
VASTNode* v_parse_class_declaration(VParser* parser);
VASTNode* v_parse_function_declaration(VParser* parser);
VASTNode* v_parse_statement(VParser* parser);
VASTNode* v_parse_expression(VParser* parser);
VASTNode* v_parse_assignment(VParser* parser);
VASTNode* v_parse_if_statement(VParser* parser);
VASTNode* v_parse_for_statement(VParser* parser);
VASTNode* v_parse_while_statement(VParser* parser);
VASTNode* v_parse_return_statement(VParser* parser);
VASTNode* v_parse_body(VParser* parser);
VASTNode* v_parse_decorator(VParser* parser);
Vector* v_parse_decorators(VParser* parser);
VASTNode* v_parse_logical_or(VParser* parser);
VASTNode* v_parse_logical_and(VParser* parser);
VASTNode* v_parse_equality(VParser* parser);
VASTNode* v_parse_comparison(VParser* parser);
VASTNode* v_parse_term(VParser* parser);
VASTNode* v_parse_factor(VParser* parser);
VASTNode* v_parse_unary(VParser* parser);
VASTNode* v_parse_primary(VParser* parser);
VASTNode* v_parse_literal(VParser* parser);
VASTNode* v_parse_variable(VParser* parser);
VASTNode* v_parse_function_call(VParser* parser);
VASTNode* v_parse_attribute_access(VParser* parser);
VType v_parse_type_annotation(VParser* parser);
Vector* v_parse_parameter_list(VParser* parser);

VASTNode* v_parse_source(VParser* parser)
{
    while(v_parser_match(parser, VTokenKind_Newline))
    {
        continue;
    }

    Vector* decls = vector_new(32, sizeof(VASTNode*));

    while(!v_parser_is_at_end(parser))
    {
        while(v_parser_match(parser, VTokenKind_Newline))
        {
            continue;
        }

        if(v_parser_is_at_end(parser))
        {
            break;
        }

        VASTNode* decl = v_parse_declaration(parser);

        if(decl == NULL)
        {
            return NULL;
        }

        vector_push_back(decls, &decl);
    }

    return v_ast_new_source(parser->ast, decls);
}

VASTNode* v_parse_declaration(VParser* parser) 
{
    /* 
        Decorator 
        TODO: Parse more than one decorator
    */

    if(v_parser_check_delimiter(parser, VDelimiter_At)) 
    {
        VASTNode* decorator = v_parse_decorator(parser);

        if(v_parser_check_keyword(parser, VKeyword_Class)) 
        {
            VASTClass* class_decl = (VASTClass*)v_parse_class_declaration(parser);

            class_decl->decorators = vector_new(1, sizeof(VASTNode*));
            vector_push_back(class_decl->decorators, &decorator);

            return (VASTNode*)class_decl;
        }
        else if(v_parser_check_keyword(parser, VKeyword_Def)) 
        {
            VASTFunction* func_decl = (VASTFunction*)v_parse_function_declaration(parser);

            func_decl->decorators = vector_new(1, sizeof(VASTNode*));
            vector_push_back(func_decl->decorators, &decorator);

            return (VASTNode*)func_decl;
        } 
        else 
        {
            v_parser_set_error(parser, "Expecting class or function declaration after decorator");
            return NULL;
        }
    }
    
    if(v_parser_check_keyword(parser, VKeyword_Class)) 
    {
        return v_parse_class_declaration(parser);
    }
    
    if(v_parser_check_keyword(parser, VKeyword_Def)) 
    {
        return v_parse_function_declaration(parser);
    }
    
    return v_parse_statement(parser);
}

VASTNode* v_parse_class_declaration(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_function_declaration(VParser* parser)
{
    if(!v_parser_consume_keyword(parser, VKeyword_Def, "Expected \"def\" keyword for function declaration"))
    {
        return NULL;
    }

    VToken* name_token = v_parser_consume(parser, VTokenKind_Identifier, "Expected a function name after \"def\"");

    if(name_token == NULL)
    {
        return NULL;
    }

    String name = string_newf("%*.s", name_token->length, name_token->start);

    Vector* params = v_parse_parameter_list(parser);

    if(params == NULL)
    {
        string_free(name);
        return NULL;
    }

    VType return_type = VType_Unknown;

    if(v_parser_match_delimiter(parser, VDelimiter_RightArrow))
    {
        return_type = v_parse_type_annotation(parser);

        if(parser->error != NULL)
        {
            string_free(name);

            for(uint32_t i = 0; i < vector_size(params); i++)
            {
                string_free(((VASTArgument*)vector_at(params, i))->name);
            }

            vector_free(params);
            return NULL;
        }
    }

    if(!v_parser_consume_delimiter(parser, VDelimiter_Colon, "Expected \":\" after function signature"))
    {
        string_free(name);

        for(uint32_t i = 0; i < vector_size(params); i++)
        {
            string_free(((VASTArgument*)vector_at(params, i))->name);
        }

        vector_free(params);
        return NULL;
    }

    VASTNode* body = v_parse_body(parser);

    if(body == NULL)
    {
        string_free(name);
        vector_free(params);
        return NULL;
    }

    return v_ast_new_function(parser->ast, name, params, return_type, body);
}

VASTNode* v_parse_statement(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_expression(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_assignment(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_if_statement(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_for_statement(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_while_statement(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_return_statement(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_body(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_decorator(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_logical_or(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_logical_and(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_equality(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_comparison(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_term(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_factor(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_unary(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_primary(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_literal(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_variable(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_function_call(VParser* parser)
{
    return NULL;
}

VASTNode* v_parse_attribute_access(VParser* parser)
{
    return NULL;
}

VType v_parse_type_annotation(VParser* parser)
{
    VToken* type_token = v_parser_peek(parser);
    return VType_Unknown;
}

Vector* v_parse_parameter_list(VParser* parser) 
{
    Vector* params = vector_new(4, sizeof(VASTNode*));

    if(!v_parser_consume_delimiter(parser, VDelimiter_LParen, "Expected \"(\" after function name")) 
    {
        vector_free(params);
        return NULL;
    }

    if (!v_parser_check_delimiter(parser, VDelimiter_RParen)) 
    {
        do {
            VToken* param_token = v_parser_consume(parser, VTokenKind_Identifier, "Expected parameter name");

            if(param_token == NULL) 
            {
                for(uint32_t i = 0; i < vector_size(params); i++)
                {
                    string_free(((VASTArgument*)vector_at(params, i))->name);
                }

                vector_free(params);
                return NULL;
            }

            String param_name = string_newf("%.*s", (int)param_token->length, param_token->start);

            VType param_type = VType_Unknown;
            VASTNode* default_value = NULL;

            if(v_parser_match_delimiter(parser, VDelimiter_Colon)) 
            {
                param_type = v_parse_type_annotation(parser);

                if(parser->error != NULL) 
                { 
                    string_free(param_name);

                    for(uint32_t i = 0; i < vector_size(params); i++)
                    {
                        string_free(((VASTArgument*)vector_at(params, i))->name);
                    }

                    vector_free(params);
                    return NULL;
                }
            }

            if(v_parser_match_operator(parser, VOperator_Assign)) 
            {
                default_value = v_parse_expression(parser);

                if(default_value == NULL) 
                { 
                    string_free(param_name);

                    for(uint32_t i = 0; i < vector_size(params); i++)
                    {
                        string_free(((VASTArgument*)vector_at(params, i))->name);
                    }

                    vector_free(params);
                    return NULL;
                }
            }

            VASTNode* param_node = v_ast_new_argument(parser->ast, param_name, param_type, default_value);
            vector_push_back(params, param_node);

        } while (v_parser_match_delimiter(parser, VDelimiter_Comma));
    }

    if(!v_parser_consume_delimiter(parser, VDelimiter_RParen, "Expected \")\" after parameters")) 
    {
        for(uint32_t i = 0; i < vector_size(params); i++)
        {
            string_free(((VASTArgument*)vector_at(params, i))->name);
        }

        vector_free(params); 
        return NULL;
    }

    return params;
}

bool v_ast_from_tokens(VAST* ast, Vector* tokens)
{
    if(ast == NULL || tokens == NULL)
    {
        return false;
    }

    VParser parser = {
        tokens,
        0,
        ast,
        NULL,
    };

    ast->root = v_parse_source(&parser);

    if(parser.error != NULL)
    {
        ast->error = parser.error;
        return false;
    }

    return true;
}

/* VAST Destruction */

void v_ast_destroy_node(VASTNode* node)
{
    if(node == NULL)
    {
        return;
    }

    const VASTNodeType type = node->type;

    switch(type)
    {
        case VASTNodeType_VASTSource: 
            v_ast_destroy_source(node);
            break;
        case VASTNodeType_VASTClass: 
            v_ast_destroy_class(node);
            break;
        case VASTNodeType_VASTFunction: 
            v_ast_destroy_function(node);
            break;
        case VASTNodeType_VASTBody: 
            v_ast_destroy_body(node);
            break;
        case VASTNodeType_VASTFor: 
            v_ast_destroy_for(node);
            break;
        case VASTNodeType_VASTIf: 
            v_ast_destroy_if(node);
            break;
        case VASTNodeType_VASTReturn: 
            v_ast_destroy_return(node);
            break;
        case VASTNodeType_VASTAssignment: 
            v_ast_destroy_assignment(node);
            break;
        case VASTNodeType_VASTUnOp: 
            v_ast_destroy_unop(node);
            break;
        case VASTNodeType_VASTBinOp: 
            v_ast_destroy_binop(node);
            break;
        case VASTNodeType_VASTTernOp: 
            v_ast_destroy_ternop(node);
            break;
        case VASTNodeType_VASTDecorator: 
            v_ast_destroy_decorator(node);
            break;
        case VASTNodeType_VASTAttribute: 
            v_ast_destroy_attribute(node);
            break;
        case VASTNodeType_VASTVariable: 
            v_ast_destroy_variable(node);
            break;
        case VASTNodeType_VASTArgument: 
            v_ast_destroy_argument(node);
            break;
        case VASTNodeType_VASTLiteral: 
            v_ast_destroy_literal(node);
            break;
        case VASTNodeType_VASTFCall: 
            v_ast_destroy_fcall(node);
            break;
        default:
            break;
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

        v_ast_destroy_node(ast->root);

        free(ast);

        ast = NULL;
    }
}

void v_ast_vector_free_callback(void* element)
{
    VASTNode* node = *(VASTNode**)element;

    v_ast_destroy_node(node);
}

/*******************************/
/* VASTNodes functions */

VASTNode* v_ast_new_source(VAST* ast, 
                           Vector* decls) 
{
    VASTSource source = {
        { VASTNodeType_VASTSource },
        decls
    };

    void* addr = arena_push(&ast->data, &source, sizeof(VASTSource));

    return (VASTNode*)addr;
}

void v_ast_destroy_source(VASTNode* source)
{
    VASTSource* src = VAST_CAST(VASTSource, source);
    VASSERT_TYPE(VASTSource, src);

    vector_free_with_dtor(src->decls, v_ast_vector_free_callback);
}

VASTNode* v_ast_new_class(VAST* ast,
                          const String name,
                          Vector* bases,
                          Vector* attributes,
                          Vector* functions) 
{
    VASTClass class_node = {
        { VASTNodeType_VASTClass },
        name,
        bases,
        attributes,
        functions,
        NULL,
    };

    void* addr = arena_push(&ast->data, &class_node, sizeof(VASTClass));

    return (VASTNode*)addr;
}

void v_ast_destroy_class(VASTNode* class)
{
    VASTClass* cls = VAST_CAST(VASTClass, class);
    VASSERT_TYPE(VASTClass, cls);

    string_free(cls->name);

    vector_free_with_dtor(cls->bases, v_ast_vector_free_callback);
    vector_free_with_dtor(cls->attributes, v_ast_vector_free_callback);
    vector_free_with_dtor(cls->functions, v_ast_vector_free_callback);
    vector_free_with_dtor(cls->decorators, v_ast_vector_free_callback);
}

VASTNode* v_ast_new_function(VAST* ast,
                             const String name,
                             Vector* params,
                             VType return_type,
                             VASTBody* body) 
{
    VASTFunction func = {
        { VASTNodeType_VASTFunction },
        name,
        params,
        body,
        return_type,
        NULL,
    };

    void* addr = arena_push(&ast->data, &func, sizeof(VASTFunction));

    return (VASTNode*)addr;
}

void v_ast_destroy_function(VASTNode* function)
{
    VASTFunction* func = VAST_CAST(VASTFunction, function);
    VASSERT_TYPE(VASTFunction, func);

    string_free(func->name);

    v_ast_destroy_node(func->body);
    vector_free_with_dtor(func->params, v_ast_vector_free_callback);
    vector_free_with_dtor(func->decorators, v_ast_vector_free_callback);
}

VASTNode* v_ast_new_body(VAST* ast,
                         Vector* stmts)
{
    VASTBody body = {
        { VASTNodeType_VASTBody },
        stmts,
    };

    void* addr = arena_push(&ast->data, &body, sizeof(VASTBody));

    return (VASTNode*)addr;
}

void v_ast_destroy_body(VASTNode* body)
{
    VASTBody* b = VAST_CAST(VASTBody, body);
    VASSERT_TYPE(VASTBody, b);

    vector_free_with_dtor(b->stmts, v_ast_vector_free_callback);
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

void v_ast_destroy_for(VASTNode* for_)
{
    VASTFor* f = VAST_CAST(VASTFor, for_);
    VASSERT_TYPE(VASTFor, f);
    
    v_ast_destroy_node(f->target);
    v_ast_destroy_node(f->cond);
    v_ast_destroy_node(f->body);
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

void v_ast_destroy_if(VASTNode* if_)
{
    VASTIf* i = VAST_CAST(VASTIf, if_);
    VASSERT_TYPE(VASTIf, i);

    v_ast_destroy_node(i->condition);
    v_ast_destroy_node(i->body);
    v_ast_destroy_node(i->else_node);
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

void v_ast_destroy_return(VASTNode* ret)
{
    VASTReturn* r = VAST_CAST(VASTReturn, ret);
    VASSERT_TYPE(VASTReturn, r);

    v_ast_destroy_node(r->value);
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

void v_ast_destroy_assignment(VASTNode* assignment)
{
    VASTAssignment* assign = VAST_CAST(VASTAssignment, assignment);
    VASSERT_TYPE(VASTAssignment, assign);

    v_ast_destroy_node(assign->target);
    v_ast_destroy_node(assign->value);
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

void v_ast_destroy_unop(VASTNode* unop)
{
    VASTUnOp* op = VAST_CAST(VASTUnOp, unop);
    VASSERT_TYPE(VASTUnOp, op);

    v_ast_destroy_node(op->operand);
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

void v_ast_destroy_binop(VASTNode* binop)
{
    VASTBinOp* op = VAST_CAST(VASTBinOp, binop);
    VASSERT_TYPE(VASTUnOp, op);

    v_ast_destroy_node(op->left);
    v_ast_destroy_node(op->right);
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

void v_ast_destroy_ternop(VASTNode* ternop)
{
    VASTTernOp* op = VAST_CAST(VASTTernOp, ternop);
    VASSERT_TYPE(VASTUnOp, op);

    v_ast_destroy_node(op->condition);
    v_ast_destroy_node(op->if_expr);
    v_ast_destroy_node(op->else_expr);
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

void v_ast_destroy_decorator(VASTNode* decorator)
{
    VASTDecorator* dec = VAST_CAST(VASTDecorator, decorator);
    VASSERT_TYPE(VASTDecorator, dec);

    string_free(dec->name);
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

void v_ast_destroy_attribute(VASTNode* attribute)
{
    VASTAttribute* attr = VAST_CAST(VASTAttribute, attribute);
    VASSERT_TYPE(VASTAttribute, attr);

    string_free(attr->name);
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

void v_ast_destroy_variable(VASTNode* variable)
{
    VASTVariable* var = VAST_CAST(VASTVariable, variable);
    VASSERT_TYPE(VASTVariable, var);

    string_free(var->name);
}

VASTNode* v_ast_new_argument(VAST* ast,
                             const String name,
                             const VType type,
                             VASTNode* default_value)
{
    VASTArgument arg = {
        { VASTNodeType_VASTArgument },
        name,
        type,
        default_value,
    };

    void* addr = arena_push(&ast->data, &arg, sizeof(VASTArgument));

    return (VASTNode*)addr;
}

void v_ast_destroy_argument(VASTNode* argument)
{
    VASTArgument* arg = VAST_CAST(VASTArgument, argument);
    VASSERT_TYPE(VASTArgument, arg);

    string_free(arg->name);
    
    v_ast_destroy_node(arg->default_value);
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
                                 Vector* elements)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_List;
    lit.value.list_val.elements = elements;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_dict(VAST* ast,
                                 Vector* keys,
                                 Vector* values)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Dict;
    lit.value.dict_val.keys = keys;
    lit.value.dict_val.values = values;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_tuple(VAST* ast,
                                  Vector* elements)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Tuple;
    lit.value.tuple_val.elements = elements;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_set(VAST* ast,
                                Vector* elements)
{
    VASTLiteral lit = { { VASTNodeType_VASTLiteral } };
    lit.lit_type = VType_Set;
    lit.value.set_val.elements = elements;

    void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

    return (VASTNode*)addr;
}

void v_ast_destroy_literal(VASTNode* literal)
{
    VASTLiteral* lit = VAST_CAST(VASTLiteral, literal);
    VASSERT_TYPE(VASTLiteral, lit);

    switch(lit->lit_type)
    {
        case VType_String:
            string_free(lit->value.str_val);
            break;
        case VType_List:
            vector_free_with_dtor(lit->value.list_val.elements, v_ast_vector_free_callback);
            break;
        case VType_Dict:
            vector_free_with_dtor(lit->value.dict_val.keys, v_ast_vector_free_callback);
            vector_free_with_dtor(lit->value.dict_val.values, v_ast_vector_free_callback);
            break;
        case VType_Tuple:
            vector_free_with_dtor(lit->value.tuple_val.elements, v_ast_vector_free_callback);
            break;
        case VType_Set:
            vector_free_with_dtor(lit->value.set_val.elements, v_ast_vector_free_callback);
            break;
    }
}

VASTNode* v_ast_new_fcall(VAST* ast,
                          const String name, 
                          Vector* args,
                          Vector* kwarg_names,
                          Vector* kwarg_values)
{
    VASTFCall fcall = {
        { VASTNodeType_VASTFCall },
        name,
        args,
        {
            kwarg_names,
            kwarg_values,
        }
    };

    void* addr = arena_push(&ast->data, &fcall, sizeof(VASTFCall));

    return (VASTNode*)addr;
}

void v_ast_vector_free_destroy_string(void* element)
{
    String to_free = *(String*)element;
    string_free(*to_free);
}

void v_ast_destroy_fcall(VASTNode* fcall)
{
    VASTFCall* f = VAST_CAST(VASTFCall, fcall);
    VASSERT_TYPE(VASTFCall, f);

    string_free(f->function_name);

    vector_free_with_dtor(f->args, v_ast_vector_free_callback);
    vector_free_with_dtor(f->kwargs.names, v_ast_vector_free_destroy_string);
    vector_free_with_dtor(f->kwargs.values, v_ast_vector_free_callback);
}