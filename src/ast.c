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
     VAST* ast = (VAST*)calloc(1, sizeof(VAST));

     arena_init(&ast->data, 1024);
     ast->root = NULL;
     ast->error = NULL;

     return ast;
}

/* VAST Callbacks */

void v_ast_vector_free_callback(void* element)
{
     VASTNode* node = *(VASTNode**)element;

     v_ast_destroy_node(node);
}

void v_ast_vector_free_destroy_string(void* element)
{
     String to_free = *(String*)element;
     string_free(to_free);
}

/* VAST Debugging */

#define VASSERT_TYPE(__type__, __node__)                                                           \
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

                    printf("Source (%zu declarations)\n", vector_size(source->decls));

                    for(uint32_t i = 0; i < vector_size(source->decls); i++)
                    {
                         v_ast_node_debug(*(VASTNode**)vector_at(source->decls, i),
                                          indent_level + 1);
                    }

                    break;
               }

          case VASTNodeType_VASTClass:
               {
                    VASTClass* class = VAST_CAST(VASTClass, node);
                    VASSERT_TYPE(VASTClass, class);

                    printf("Class \"%s\" (%zu bases, %zu attributes, %zu functions, %zu "
                           "decorators)\n",
                           class->name,
                           class->bases != NULL ? vector_size(class->bases) : 0,
                           class->attributes != NULL ? vector_size(class->attributes) : 0,
                           class->functions != NULL ? vector_size(class->functions) : 0,
                           class->decorators != NULL ? vector_size(class->decorators) : 0);

                    if(class->bases != NULL && vector_size(class->bases) > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Bases:\n");

                         for(uint32_t i = 0; i < vector_size(class->bases); i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(class->bases, i),
                                               indent_level + 2);
                         }
                    }

                    if(class->attributes != NULL && vector_size(class->attributes) > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Attributes:\n");

                         for(uint32_t i = 0; i < vector_size(class->attributes); i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(class->attributes, i),
                                               indent_level + 2);
                         }
                    }

                    if(class->functions != NULL && vector_size(class->functions) > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Functions:\n");

                         for(uint32_t i = 0; i < vector_size(class->functions); i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(class->functions, i),
                                               indent_level + 2);
                         }
                    }

                    if(class->decorators != NULL && vector_size(class->decorators) > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Decorators:\n");

                         for(uint32_t i = 0; i < vector_size(class->functions); i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(class->functions, i),
                                               indent_level + 2);
                         }
                    }

                    break;
               }

          case VASTNodeType_VASTFunction:
               {
                    VASTFunction* func = VAST_CAST(VASTFunction, node);
                    VASSERT_TYPE(VASTFunction*, func);

                    printf("Function \"%s\" (%zu parameters, return_type: %s, %zu "
                           "decorators)\n",
                           func->name,
                           vector_size(func->params),
                           v_type_to_string(func->return_type),
                           func->decorators != NULL ? vector_size(func->decorators) : 0);

                    if(func->params != NULL && vector_size(func->params) > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Parameters:\n");

                         for(uint32_t i = 0; i < vector_size(func->params); i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(func->params, i),
                                               indent_level + 2);
                         }
                    }

                    if(func->decorators != NULL && vector_size(func->decorators) > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Decorators:\n");

                         for(uint32_t i = 0; i < vector_size(func->decorators); i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(func->decorators, i),
                                               indent_level + 2);
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

                    printf("Body (%zu statements)\n", vector_size(body->stmts));

                    for(uint32_t i = 0; i < vector_size(body->stmts); i++)
                    {
                         v_ast_node_debug(*(VASTNode**)vector_at(body->stmts, i), indent_level + 1);
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

                    printf("Assignment (op: %d, type: %s)\n",
                           assign->op,
                           v_type_to_string(assign->type));

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

                    printf("Attribute \"%s\" (type: %s)\n",
                           attr->name,
                           v_type_to_string(attr->type));

                    break;
               }

          case VASTNodeType_VASTVariable:
               {
                    VASTVariable* var = VAST_CAST(VASTVariable, node);
                    VASSERT_TYPE(VASTVariable*, var);

                    printf("Variable \"%s\" (type: %s)\n", var->name, v_type_to_string(var->type));

                    break;
               }

          case VASTNodeType_VASTParameter:
               {
                    VASTParameter* arg = VAST_CAST(VASTParameter, node);
                    VASSERT_TYPE(VASTParameter, arg);

                    printf("Parameter \"%s\" (type: %s, has_default_value: %s)\n",
                           arg->name,
                           v_type_to_string(arg->type),
                           arg->default_value == NULL ? "false" : "true");

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
                              printf("Int: %zi\n", lit->value.int_val);
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
                              printf("List (%zu elements)\n",
                                     vector_size(lit->value.list_val.elements));

                              for(uint32_t i = 0; i < vector_size(lit->value.list_val.elements);
                                  i++)
                              {
                                   v_ast_node_debug(*(VASTNode**)vector_at(
                                                                   lit->value.list_val.elements,
                                                                   i),
                                                    indent_level + 1);
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
                                        v_ast_node_debug(*(VASTNode**)vector_at(
                                                                        lit->value.dict_val.keys,
                                                                        i),
                                                         indent_level + 2);

                                        print_indent(indent_level + 1);
                                        printf("Value %zu:\n", i);
                                        v_ast_node_debug(*(VASTNode**)vector_at(
                                                                        lit->value.dict_val.values,
                                                                        i),
                                                         indent_level + 2);
                                   }

                                   break;
                              }

                         case VType_Tuple:
                              {
                                   size_t num_elements = vector_size(lit->value.tuple_val.elements);
                                   printf("Tuple (%zu elements)\n", num_elements);

                                   for(size_t i = 0; i < num_elements; i++)
                                   {
                                        v_ast_node_debug(
                                                       *(VASTNode**)vector_at(
                                                                      lit->value.tuple_val.elements,
                                                                      i),
                                                       indent_level + 1);
                                   }

                                   break;
                              }

                         case VType_Set:
                              {
                                   size_t num_elements = vector_size(lit->value.set_val.elements);
                                   printf("Set (%zu elements)\n", num_elements);

                                   for(size_t i = 0; i < num_elements; i++)
                                   {
                                        v_ast_node_debug(*(VASTNode**)vector_at(
                                                                        lit->value.set_val.elements,
                                                                        i),
                                                         indent_level + 1);
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

                    size_t num_args = fcall->args != NULL ? vector_size(fcall->args) : 0;
                    size_t num_kwargs = fcall->kwargs.names != NULL ? vector_size(fcall->kwargs.names) : 0;

                    printf("Function Call (%zu args, %zu kwargs)\n", num_args, num_kwargs);

                    print_indent(indent_level + 1);
                    printf("Callable:\n");

                    v_ast_node_debug(fcall->callable, indent_level + 2);

                    if(num_args > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Arguments:\n");

                         for(size_t i = 0; i < num_args; i++)
                         {
                              v_ast_node_debug(*(VASTNode**)vector_at(fcall->args, i),
                                               indent_level + 2);
                         }
                    }

                    if(num_kwargs > 0)
                    {
                         print_indent(indent_level + 1);
                         printf("Keyword Arguments:\n");

                         for(size_t i = 0; i < num_kwargs; i++)
                         {
                              print_indent(indent_level + 2);
                              printf("\"%s\":\n", *(String*)vector_at(fcall->kwargs.names, i));
                              v_ast_node_debug(*(VASTNode**)vector_at(fcall->kwargs.values, i),
                                               indent_level + 3);
                         }
                    }

                    break;
               }

          case VASTNodeType_VASTAttributeAccess:
               {
                    VASTAttributeAccess* acc = VAST_CAST(VASTAttributeAccess, node);
                    VASSERT_TYPE(VASTAttributeAccess, acc);

                    printf("Attribute Access \"%s\"",
                           acc->attribute_name != NULL ? acc->attribute_name : "<NULL>");

                    print_indent(indent_level + 1);
                    printf("Object:\n");

                    v_ast_node_debug(acc->object, indent_level + 2);

                    break;
               }

          case VASTNodeType_VASTSubscript:
               {
                    VASTSubscript* sub = VAST_CAST(VASTSubscript, node);
                    VASSERT_TYPE(VASTSubscript, sub);

                    printf("Subscript Access\n");

                    print_indent(indent_level + 1);
                    printf("Value:\n");
                    v_ast_node_debug(sub->value, indent_level + 2);

                    print_indent(indent_level + 1);
                    printf("Slice/Index:\n");
                    v_ast_node_debug(sub->slice, indent_level + 2);

                    break;
               }

          case VASTNodeType_VASTSlice:
               {
                    VASTSlice* slice = VAST_CAST(VASTSlice, node);
                    VASSERT_TYPE(VASTSlice, slice);

                    printf("Slice\n");

                    if(slice->start != NULL)
                    {
                         print_indent(indent_level + 1);
                         printf("Start:\n");
                         v_ast_node_debug(slice->start, indent_level + 2);
                    }
                    else
                    {
                         print_indent(indent_level + 1);
                         printf("Start: <default>\n");
                    }

                    if(slice->stop != NULL)
                    {
                         print_indent(indent_level + 1);
                         printf("Stop:\n");
                         v_ast_node_debug(slice->stop, indent_level + 2);
                    }
                    else
                    {
                         print_indent(indent_level + 1);
                         printf("Stop: <default>\n");
                    }

                    if(slice->step != NULL)
                    {
                         print_indent(indent_level + 1);
                         printf("Step:\n");
                         v_ast_node_debug(slice->step, indent_level + 2);
                    }
                    else
                    {
                         print_indent(indent_level + 1);
                         printf("Step: <default>\n");
                    }

                    break;
               }

          case VASTNodeType_VASTPass:
               {
                    printf("Pass\n");
                    break;
               }

          case VASTNodeType_VASTBreak:
               {
                    printf("Break\n");
                    break;
               }

          case VASTNodeType_VASTContinue:
               {
                    printf("Continue\n");
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

typedef struct
{
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

VENOM_FORCE_INLINE void v_parser_set_error(VParser* parser, const char* message)
{
     if(parser->error != NULL)
     {
          return;
     }

     VToken* token = v_parser_peek(parser);
     const uint32_t line_info = token->line > 0 ? token->line : 0;

     parser->error = string_newf("Parsing error at line %d: %s", line_info, message);
}

VENOM_FORCE_INLINE VToken* v_parser_consume_kind(VParser* parser,
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

VASTNode* v_parse_source(VParser* parser);
VASTNode* v_parse_declaration(VParser* parser);
VASTNode* v_parse_class_declaration(VParser* parser);
VASTNode* v_parse_function_declaration(VParser* parser);
VASTNode* v_parse_statement(VParser* parser);
VASTNode* v_parse_simple_statement(VParser* parser);
VASTNode* v_parse_compound_statement(VParser* parser);
VASTNode* v_parse_expression_statement(VParser* parser);
VASTNode* v_parse_assignment_statement(VParser* parser, VASTNode* target);
VASTNode* v_parse_if_statement(VParser* parser);
VASTNode* v_parse_for_statement(VParser* parser);
VASTNode* v_parse_while_statement(VParser* parser);
VASTNode* v_parse_return_statement(VParser* parser);
VASTNode* v_parse_pass_statement(VParser* parser);
VASTNode* v_parse_break_statement(VParser* parser);
VASTNode* v_parse_continue_statement(VParser* parser);
VASTNode* v_parse_body(VParser* parser);
VASTNode* v_parse_decorator(VParser* parser);
Vector* v_parse_decorators(VParser* parser);
VASTNode* v_parse_expression(VParser* parser);
VASTNode* v_parse_ternary(VParser* parser);
VASTNode* v_parse_logical_or(VParser* parser);
VASTNode* v_parse_logical_and(VParser* parser);
VASTNode* v_parse_comparison(VParser* parser);
VASTNode* v_parse_bitwise_or(VParser* parser);
VASTNode* v_parse_bitwise_xor(VParser* parser);
VASTNode* v_parse_bitwise_and(VParser* parser);
VASTNode* v_parse_shift(VParser* parser);
VASTNode* v_parse_term(VParser* parser);
VASTNode* v_parse_factor(VParser* parser);
VASTNode* v_parse_unary(VParser* parser);
VASTNode* v_parse_power(VParser* parser);
VASTNode* v_parse_primary(VParser* parser);
VASTNode* v_parse_atom(VParser* parser);
VASTNode* v_parse_literal(VParser* parser);
VASTNode* v_parse_variable(VParser* parser);
VASTNode* v_parse_function_call(VParser* parser, VASTNode* callable);
VASTNode* v_parse_attribute_access(VParser* parser, VASTNode* object);
VASTNode* v_parse_subscript(VParser* parser, VASTNode* object);
VASTNode* v_parse_slice(VParser* parser);
VType v_parse_type_annotation(VParser* parser);
Vector* v_parse_parameter_list(VParser* parser);
Vector* v_parse_argument_list(VParser* parser, Vector** out_kwarg_names, Vector** out_kwarg_values);

VASTNode* v_parse_source(VParser* parser)
{
     while(v_parser_match(parser, VTokenKind_Newline) ||
           v_parser_match(parser, VTokenKind_Indent) || v_parser_match(parser, VTokenKind_Dedent))
     {
          continue;
     }

     Vector* decls = vector_new(32, sizeof(VASTNode*));

     if(decls == NULL)
     {
          v_parser_set_error(parser, "Failed to allocate vector for source declarations");
          return NULL;
     }

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
               vector_free(decls);
               return NULL;
          }

          vector_push_back(decls, &decl);
     }

     return v_ast_new_source(parser->ast, decls);
}

Vector* v_parse_decorators(VParser* parser)
{
     Vector* decorators = NULL;

     while(v_parser_check_delimiter(parser, VDelimiter_At))
     {
          if(decorators == NULL)
          {
               decorators = vector_new(2, sizeof(VASTNode*));
          }

          VASTNode* decorator = v_parse_decorator(parser);

          if(decorator == NULL)
          {
               if(decorators != NULL)
               {
                    vector_free_with_dtor(decorators, v_ast_vector_free_callback);
               }

               return NULL;
          }

          vector_push_back(decorators, &decorator);
     }

     return decorators;
}

VASTNode* v_parse_decorator(VParser* parser)
{
     if(!v_parser_consume_delimiter(parser, VDelimiter_At, "Expected \"@\" for decorator"))
     {
          return NULL;
     }

     VToken* name_token = v_parser_consume_kind(parser,
                                                VTokenKind_Identifier,
                                                "Expected decorator name after \"@\"");
     if(name_token == NULL)
     {
          return NULL;
     }

     String decorator_name = string_newf("%.*s", (int)name_token->length, name_token->start);

     /* TODO: Handle decorator arguments like @decorator(arg1, arg2) */
     if(v_parser_check_delimiter(parser, VDelimiter_LParen))
     {
          string_free(decorator_name);
          v_parser_set_error(parser, "Decorator arguments not supported yet");
          return NULL;
     }

     if(!v_parser_consume_kind(parser, VTokenKind_Newline, "Expected newline after decorator"))
     {
          string_free(decorator_name);
          return NULL;
     }

     return v_ast_new_decorator(parser->ast, decorator_name);
}

VASTNode* v_parse_declaration(VParser* parser)
{
     Vector* decorators = v_parse_decorators(parser);

     if(parser->error != NULL)
     {
          if(decorators != NULL)
          {
               vector_free(decorators);
          }

          return NULL;
     }

     VASTNode* decl = NULL;

     if(v_parser_check_keyword(parser, VKeyword_Class))
     {
          decl = v_parse_class_declaration(parser);
     }
     else if(v_parser_check_keyword(parser, VKeyword_Def))
     {
          decl = v_parse_function_declaration(parser);
     }
     else
     {
          if(decorators != NULL)
          {
               v_parser_set_error(parser,
                                  "Expected class or function definition after decorator(s)");

               if(decorators)
               {
                    vector_free_with_dtor(decorators, v_ast_vector_free_callback);
               }

               return NULL;
          }

          decl = v_parse_statement(parser);
     }

     if(decl == NULL)
     {
          if(decorators != NULL)
          {
               vector_free_with_dtor(decorators, v_ast_vector_free_callback);
          }

          return NULL;
     }

     if(decorators != NULL)
     {
          if(decl->type == VASTNodeType_VASTClass)
          {
               VASTClass* cls = VAST_CAST(VASTClass, decl);
               VASSERT_TYPE(VASTClass, cls);

               cls->decorators = decorators;
          }
          else if(decl->type == VASTNodeType_VASTFunction)
          {
               VASTFunction* func = VAST_CAST(VASTFunction, decl);
               VASSERT_TYPE(VASTFunction, func);

               func->decorators = decorators;
          }
          else
          {
               v_parser_set_error(parser, "Decorators can only be applied to functions or classes");
               vector_free_with_dtor(decorators, v_ast_vector_free_callback);
               v_ast_destroy_node(decl);
               return NULL;
          }
     }

     return decl;
}

VASTNode* v_parse_class_declaration(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_Class, "Expected \"class\" keyword"))
     {
          return NULL;
     }

     VToken* name_token =
                    v_parser_consume_kind(parser, VTokenKind_Identifier, "Expected class name");
     if(name_token == NULL)
     {
          return NULL;
     }

     String class_name = string_newf("%.*s", (int)name_token->length, name_token->start);
     if(class_name == NULL)
     {
          v_parser_set_error(parser, "Failed to allocate class name string");
          return NULL;
     }

     Vector* bases = vector_new(2, sizeof(VASTNode*));

     if(v_parser_match_delimiter(parser, VDelimiter_LParen))
     {
          if(!v_parser_check_delimiter(parser, VDelimiter_RParen))
          {
               do
               {
                    VASTNode* base_expr = v_parse_expression(parser);

                    if(base_expr == NULL)
                    {
                         string_free(class_name);
                         vector_free_with_dtor(bases, v_ast_vector_free_callback);
                         return NULL;
                    }

                    vector_push_back(bases, &base_expr);

               } while(v_parser_match_delimiter(parser, VDelimiter_Comma));
          }

          if(!v_parser_consume_delimiter(parser,
                                         VDelimiter_RParen,
                                         "Expected \")\" after base classes"))
          {
               string_free(class_name);
               vector_free_with_dtor(bases, v_ast_vector_free_callback);
               return NULL;
          }
     }

     if(!v_parser_consume_delimiter(parser,
                                    VDelimiter_Colon,
                                    "Expected \":\" after class definition header"))
     {
          string_free(class_name);
          vector_free_with_dtor(bases, v_ast_vector_free_callback);
          return NULL;
     }

     VASTNode* body_node = v_parse_body(parser);

     if(body_node == NULL)
     {
          string_free(class_name);
          vector_free_with_dtor(bases, v_ast_vector_free_callback);
          return NULL;
     }

     if(body_node->type != VASTNodeType_VASTBody)
     {
          string_free(class_name);
          vector_free_with_dtor(bases, v_ast_vector_free_callback);
          v_ast_destroy_node(body_node);
          v_parser_set_error(parser, "Internal error: Class body parsing did not return VASTBody");
          return NULL;
     }

     VASTBody* body = VAST_CAST(VASTBody, body_node);
     Vector* attributes = vector_new(8, sizeof(VASTNode*));
     Vector* functions = vector_new(8, sizeof(VASTNode*));

     for(uint32_t i = 0; i < vector_size(body->stmts); ++i)
     {
          VASTNode* stmt = *(VASTNode**)vector_at(body->stmts, i);

          if(stmt->type == VASTNodeType_VASTFunction)
          {
               vector_push_back(functions, &stmt);
          }
          else if(stmt->type == VASTNodeType_VASTAssignment)
          {
               VASTAssignment* assign = VAST_CAST(VASTAssignment, stmt);

               if(assign->target->type == VASTNodeType_VASTVariable)
               {
                    VASTVariable* var_target = VAST_CAST(VASTVariable, assign->target);
                    VASSERT_TYPE(VASTVariable, var_target);

                    String attr_name = string_copy(var_target->name);
                    VASTNode* attr_node = v_ast_new_attribute(parser->ast,
                                                              attr_name,
                                                              assign->type,
                                                              assign->value);
                    vector_push_back(attributes, &attr_node);

                    ((VASTAttribute*)attr_node)->initial_value = assign->value;
                    assign->value = NULL;
               }
               else
               {
                    /* TODO for later. For now we disallow complex assignments */
                    v_parser_set_error(parser,
                                       "Complex assignment target not allowed "
                                       "directly in class body (use methods)");

                    string_free(class_name);
                    vector_free_with_dtor(bases, v_ast_vector_free_callback);
                    v_ast_destroy_node(body_node);
                    vector_free_with_dtor(attributes, v_ast_vector_free_callback);
                    vector_free_with_dtor(functions, v_ast_vector_free_callback);

                    return NULL;
               }
          }
          else if(stmt->type == VASTNodeType_VASTClass)
          {
               /* Nested class definition as attribute (like CPython AST) */
               vector_push_back(attributes, &stmt);
          }
          else if(stmt->type == VASTNodeType_VASTPass)
          {
          }
          else if(stmt->type == VASTNodeType_VASTLiteral &&
                  ((VASTLiteral*)stmt)->lit_type == VType_String)
          {
               /* Could be docstring, ignore for now */
          }
          else
          {
               v_parser_set_error(parser, "Unexpected statement type found directly in class body");

               string_free(class_name);
               vector_free_with_dtor(bases, v_ast_vector_free_callback);
               v_ast_destroy_node(body_node);
               vector_free_with_dtor(attributes, v_ast_vector_free_callback);
               vector_free_with_dtor(functions, v_ast_vector_free_callback);
               return NULL;
          }
     }

     VASTNode* class_node = v_ast_new_class(parser->ast, class_name, bases, attributes, functions);

     vector_free(body->stmts);

     return class_node;
}

VASTNode* v_parse_function_declaration(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_Def, "Expected \"def\" keyword"))
     {
          return NULL;
     }

     VToken* name_token = v_parser_consume_kind(parser,
                                                VTokenKind_Identifier,
                                                "Expected function name");

     if(name_token == NULL)
     {
          return NULL;
     }

     String func_name = string_newf("%.*s", (int)name_token->length, name_token->start);

     Vector* params = v_parse_parameter_list(parser);

     if(params == NULL)
     {
          string_free(func_name);
          return NULL;
     }

     VType return_type = VType_Unknown;

     if(v_parser_match_delimiter(parser, VDelimiter_RightArrow))
     {
          return_type = v_parse_type_annotation(parser);

          if(parser->error)
          {
               string_free(func_name);
               vector_free_with_dtor(params, v_ast_vector_free_callback);
               return NULL;
          }
     }

     if(!v_parser_consume_delimiter(parser,
                                    VDelimiter_Colon,
                                    "Expected \":\" after function signature"))
     {
          string_free(func_name);
          vector_free_with_dtor(params, v_ast_vector_free_callback);
          return NULL;
     }

     VASTNode* body_node = v_parse_body(parser);

     if(!body_node)
     {
          string_free(func_name);
          vector_free_with_dtor(params, v_ast_vector_free_callback);
          return NULL;
     }

     if(body_node->type != VASTNodeType_VASTBody)
     {
          string_free(func_name);
          vector_free_with_dtor(params, v_ast_vector_free_callback);
          v_ast_destroy_node(body_node);
          v_parser_set_error(parser,
                             "Internal error: Function body parsing did not return VASTBody");
          return NULL;
     }

     return v_ast_new_function(parser->ast, func_name, params, return_type, (VASTBody*)body_node);
}

VASTNode* v_parse_body(VParser* parser)
{
     if(!v_parser_consume_kind(parser,
                               VTokenKind_Newline,
                               "Expected newline after \":\" before block"))
     {
          return NULL;
     }

     if(!v_parser_consume_kind(parser, VTokenKind_Indent, "Expected indent to start block"))
     {
          return NULL;
     }

     Vector* stmts = vector_new(8, sizeof(VASTNode*));

     while(!v_parser_check(parser, VTokenKind_Dedent) && !v_parser_is_at_end(parser))
     {
          while(v_parser_match(parser, VTokenKind_Newline))
          {
               continue;
          }

          if(v_parser_check(parser, VTokenKind_Dedent) || v_parser_is_at_end(parser))
          {
               break;
          }

          VASTNode* stmt = v_parse_statement(parser);

          if(stmt == NULL)
          {
               vector_free_with_dtor(stmts, v_ast_vector_free_callback);
               return NULL;
          }

          vector_push_back(stmts, &stmt);
     }

     /* TODO: add a pass node */

     if(!v_parser_is_at_end(parser) && !v_parser_check(parser, VTokenKind_EOF) &&
        !v_parser_consume_kind(parser, VTokenKind_Dedent, "Expected dedent at end of block"))
     {
          vector_free_with_dtor(stmts, v_ast_vector_free_callback);
          return NULL;
     }

     return v_ast_new_body(parser->ast, stmts);
}

VASTNode* v_parse_statement(VParser* parser)
{
     while(v_parser_match(parser, VTokenKind_Newline))
     {
          continue;
     }

     if(v_parser_check_keyword(parser, VKeyword_If) ||
        v_parser_check_keyword(parser, VKeyword_For) ||
        v_parser_check_keyword(parser, VKeyword_While) ||
        v_parser_check_keyword(parser, VKeyword_With) || // TODO
        v_parser_check_keyword(parser, VKeyword_Try))    // TODO
     {
          return v_parse_compound_statement(parser);
     }
     else
     {
          VASTNode* stmt = v_parse_simple_statement(parser);

          if(stmt == NULL)
          {
               return NULL;
          }

          /* TODO: Handle multiple statements on one line. */
          if(v_parser_match_delimiter(parser, VDelimiter_SemiColon))
          {
               v_parser_set_error(parser,
                                  "Multiple statements on one line (using "
                                  "\";\") not fully supported yet");
               v_ast_destroy_node(stmt);

               return NULL;
          }

          if(!v_parser_match(parser, VTokenKind_EOF) &&
             !v_parser_match(parser, VTokenKind_Newline) && !v_parser_is_at_end(parser) &&
             !v_parser_check(parser, VTokenKind_Dedent))
          {
               v_parser_set_error(parser,
                                  "Expected newline or end of block after simple statement");
               v_ast_destroy_node(stmt);
               return NULL;
          }

          return stmt;
     }
}

VASTNode* v_parse_compound_statement(VParser* parser)
{
     if(v_parser_check_keyword(parser, VKeyword_If))
     {
          return v_parse_if_statement(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_For))
     {
          return v_parse_for_statement(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_While))
     {
          return v_parse_while_statement(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_Def))
     {
          /* Nested function */
          return v_parse_function_declaration(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_Class))
     {
          /* Nested class */
          return v_parse_class_declaration(parser);
     }
     /* TODO add With and Try */

     v_parser_set_error(parser, "Internal error: Unhandled compound statement");

     return NULL;
}

VASTNode* v_parse_simple_statement(VParser* parser)
{
     if(v_parser_check_keyword(parser, VKeyword_Return))
     {
          return v_parse_return_statement(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_Pass))
     {
          return v_parse_pass_statement(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_Break))
     {
          return v_parse_break_statement(parser);
     }
     if(v_parser_check_keyword(parser, VKeyword_Continue))
     {
          return v_parse_continue_statement(parser);
     }

     /* 
          TODO: Add other simple statement keywords: 
          del, assert, global, nonlocal,
          import, from, raise, yield...
     */

     return v_parse_expression_statement(parser);
}

VASTNode* v_parse_expression_statement(VParser* parser)
{
     VASTNode* expr = v_parse_expression(parser);

     if(expr == NULL)
     {
          return NULL;
     }

     VOperator assign_op = VOperator_Unknown;
     VType target_type = VType_Unknown;

     bool annotated = false;

     if(v_parser_match_delimiter(parser, VDelimiter_Colon))
     {
          if(expr->type != VASTNodeType_VASTVariable &&
             expr->type != VASTNodeType_VASTAttributeAccess &&
             expr->type != VASTNodeType_VASTSubscript)
          {
               v_parser_set_error(parser,
                                  "Annotated assignment target must be a "
                                  "variable, attribute, or subscript");
               v_ast_destroy_node(expr);

               return NULL;
          }

          target_type = v_parse_type_annotation(parser);

          if(parser->error != NULL)
          {
               v_ast_destroy_node(expr);
               return NULL;
          }

          annotated = true;

          if(v_parser_match_operator(parser, VOperator_Assign))
          {
               assign_op = VOperator_Assign;
          }
          else
          {
               v_parser_set_error(parser, "Expected \"=\" after type annotation in statement");
               v_ast_destroy_node(expr);

               return NULL;
          }
     }
     else
     {
          if(v_parser_match_operator(parser, VOperator_Assign))
               assign_op = VOperator_Assign;
          else if(v_parser_match_operator(parser, VOperator_AdditionAssign))
               assign_op = VOperator_AdditionAssign;
          else if(v_parser_match_operator(parser, VOperator_SubtractionAssign))
               assign_op = VOperator_SubtractionAssign;
          else if(v_parser_match_operator(parser, VOperator_MultiplicationAssign))
               assign_op = VOperator_MultiplicationAssign;
          else if(v_parser_match_operator(parser, VOperator_DivisionAssign))
               assign_op = VOperator_DivisionAssign;
          else if(v_parser_match_operator(parser, VOperator_ModulusAssign))
               assign_op = VOperator_ModulusAssign;
          else if(v_parser_match_operator(parser, VOperator_FloorDivisionAssign))
               assign_op = VOperator_FloorDivisionAssign;
          else if(v_parser_match_operator(parser, VOperator_ExponentiationAssign))
               assign_op = VOperator_ExponentiationAssign;
          else if(v_parser_match_operator(parser, VOperator_BitwiseAndAssign))
               assign_op = VOperator_BitwiseAndAssign;
          else if(v_parser_match_operator(parser, VOperator_BitwiseOrAssign))
               assign_op = VOperator_BitwiseOrAssign;
          else if(v_parser_match_operator(parser, VOperator_BitwiseXorAssign))
               assign_op = VOperator_BitwiseXorAssign;
          else if(v_parser_match_operator(parser, VOperator_BitwiseLShiftAssign))
               assign_op = VOperator_BitwiseLShiftAssign;
          else if(v_parser_match_operator(parser, VOperator_BitwiseRShiftAssign))
               assign_op = VOperator_BitwiseRShiftAssign;
          /* 
               TODO: Add other augmented assignments or just consume the operator 
                     specifying it needs to be an assignment operator 
          */
     }

     if(assign_op != VOperator_Unknown)
     {
          VASTNode* target = expr;
          VASTNode* value = v_parse_expression(parser);

          if(value == NULL)
          {
               v_ast_destroy_node(target);
               return NULL;
          }

          return v_ast_new_assignment(parser->ast, target, value, assign_op, target_type);
     }
     else
     {
          return expr;
     }
}

VASTNode* v_parse_if_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_If, "Expected \"if\" keyword"))
     {
          return NULL;
     }

     VASTNode* condition = v_parse_expression(parser);
     if(condition == NULL)
     {
          return NULL;
     }

     if(!v_parser_consume_delimiter(parser, VDelimiter_Colon, "Expected \":\" after if condition"))
     {
          v_ast_destroy_node(condition);
          return NULL;
     }

     VASTNode* body_node = v_parse_body(parser);

     if(body_node == NULL)
     {
          v_ast_destroy_node(condition);
          return NULL;
     }

     if(body_node->type != VASTNodeType_VASTBody)
     {
          v_ast_destroy_node(condition);
          v_ast_destroy_node(body_node);
          return NULL;
     }

     VASTNode* else_node = NULL;
     VASTNode* current_if_else_chain_end = NULL;

     while(v_parser_match_keyword(parser, VKeyword_Elif))
     {
          VASTNode* elif_condition = v_parse_expression(parser);

          if(elif_condition == NULL)
          {
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);
               return NULL;
          }

          if(!v_parser_consume_delimiter(parser,
                                         VDelimiter_Colon,
                                         "Expected \":\" after elif condition"))
          {
               v_ast_destroy_node(elif_condition);
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);

               return NULL;
          }

          VASTNode* elif_body_node = v_parse_body(parser);

          if(elif_body_node == NULL)
          {
               v_ast_destroy_node(elif_condition);
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);
               return NULL;
          }

          if(elif_body_node->type != VASTNodeType_VASTBody)
          { 
               v_parser_set_error(parser, "Internal error while parsing elif block");
               v_ast_destroy_node(elif_condition);
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);
               return NULL;
          }

          VASTNode* new_elif_if_node = v_ast_new_if(parser->ast,
                                                    elif_condition,
                                                    (VASTBody*)elif_body_node,
                                                    NULL);

          if(else_node == NULL)
          { 
               else_node = new_elif_if_node;
               current_if_else_chain_end = else_node;
          }
          else
          {
               VASSERT_TYPE(VASTIf, current_if_else_chain_end);
               ((VASTIf*)current_if_else_chain_end)->else_node = new_elif_if_node;

               current_if_else_chain_end = new_elif_if_node;
          }
     }

     if(v_parser_match_keyword(parser, VKeyword_Else))
     {
          if(!v_parser_consume_delimiter(parser,
                                         VDelimiter_Colon,
                                         "Expected \":\" after else keyword"))
          {
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);
               return NULL;
          }

          VASTNode* final_else_body = v_parse_body(parser);

          if(!final_else_body)
          {
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);
               return NULL;
          }

          if(final_else_body->type != VASTNodeType_VASTBody)
          { 
               v_parser_set_error(parser, "Internal error while parsing else block");
               v_ast_destroy_node(condition);
               v_ast_destroy_node(body_node);
               v_ast_destroy_node(else_node);
               return NULL;
          }

          if(else_node == NULL)
          {
               else_node = final_else_body;
          }
          else
          {
               VASSERT_TYPE(VASTIf, current_if_else_chain_end);
               ((VASTIf*)current_if_else_chain_end)->else_node = final_else_body;
          }
     }

     return v_ast_new_if(parser->ast, condition, (VASTBody*)body_node, else_node);
}

VASTNode* v_parse_for_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_For, "Expected \"for\" keyword"))
          return NULL;

     VASTNode* target = v_parse_expression(parser);

     if(target == NULL)
     {
          return NULL;
     }

     /* TODO: Validate target is a valid loop target (identifier, tuple/list..) */

     if(!v_parser_consume_keyword(parser, VKeyword_In, "Expected \"in\" keyword after for target"))
     {
          v_ast_destroy_node(target);
          return NULL;
     }

     VASTNode* iterable = v_parse_expression(parser);

     if(iterable == NULL)
     {
          v_ast_destroy_node(target);
          return NULL;
     }

     if(!v_parser_consume_delimiter(parser,
                                    VDelimiter_Colon,
                                    "Expected \":\" after for statement header"))
     {
          v_ast_destroy_node(target);
          v_ast_destroy_node(iterable);
          return NULL;
     }

     VASTNode* body_node = v_parse_body(parser);

     if(body_node == NULL)
     {
          v_ast_destroy_node(target);
          v_ast_destroy_node(iterable);
          return NULL;
     }

     if(body_node->type != VASTNodeType_VASTBody)
     { 
          v_parser_set_error(parser, "Internal error while parsing for block");
          v_ast_destroy_node(target);
          v_ast_destroy_node(iterable);
          return NULL;
     }

     return v_ast_new_for(parser->ast, false, target, iterable, (VASTBody*)body_node);
}

VASTNode* v_parse_while_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_While, "Expected \"while\" keyword"))
     {
          return NULL;
     }

     VASTNode* condition = v_parse_expression(parser);

     if(condition == NULL)
     {
          return NULL;
     }

     if(!v_parser_consume_delimiter(parser, VDelimiter_Colon, "Expected \":\" after while condition"))
     {
          v_ast_destroy_node(condition);
          return NULL;
     }

     VASTNode* body_node = v_parse_body(parser);

     if(body_node == NULL)
     {
          v_ast_destroy_node(condition);
          return NULL;
     }

     if(body_node->type != VASTNodeType_VASTBody)
     { 
          v_parser_set_error(parser, "Internal error while parsing while block");
          v_ast_destroy_node(condition);
          return NULL;
     }

     return v_ast_new_for(parser->ast,
                          true,
                          NULL,
                          condition,
                          (VASTBody*)body_node);
}

VASTNode* v_parse_return_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_Return, "Expected \"return\" keyword"))
     {
          return NULL;
     }

     VASTNode* value = NULL;

     if(!v_parser_check(parser, VTokenKind_Newline) &&
        !v_parser_check_delimiter(parser, VDelimiter_SemiColon) &&
        !v_parser_check(parser, VTokenKind_Dedent) && !v_parser_is_at_end(parser))
     {
          value = v_parse_expression(parser);

          if(value == NULL)
          {
               return NULL;
          }
     }

     return v_ast_new_return(parser->ast, value);
}

VASTNode* v_parse_pass_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_Pass, "Expected \"pass\" keyword"))
     {
          return NULL;
     }

     return v_ast_new_pass(parser->ast);
}

VASTNode* v_parse_break_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_Break, "Expected \"break\" keyword"))
     {
          return NULL;
     }

     return v_ast_new_break(parser->ast);
}

VASTNode* v_parse_continue_statement(VParser* parser)
{
     if(!v_parser_consume_keyword(parser, VKeyword_Continue, "Expected \"continue\" keyword"))
     {
          return NULL;
     }

     return v_ast_new_continue(parser->ast);
}

VASTNode* v_parse_expression(VParser* parser)
{
     /* TODO: handle lambda expressions */
     if(v_parser_check_keyword(parser, VKeyword_Lambda))
     {
          v_parser_set_error(parser, "Lambda expressions not implemented yet");
          return NULL;
     }

     return v_parse_ternary(parser);
}

VASTNode* v_parse_ternary(VParser* parser)
{
     VASTNode* expr_if_true = v_parse_logical_or(parser);

     if(expr_if_true == NULL)
     {
          return NULL;
     }

     if(v_parser_match_keyword(parser, VKeyword_If))
     {
          VASTNode* condition = v_parse_logical_or(parser);

          if(condition == NULL)
          {
               v_ast_destroy_node(expr_if_true);
               return NULL;
          }

          if(!v_parser_consume_keyword(parser,
                                       VKeyword_Else,
                                       "Expected \"else\" after \"if\" in ternary expression"))
          {
               v_ast_destroy_node(expr_if_true);
               v_ast_destroy_node(condition);
               return NULL;
          }

          VASTNode* expr_if_false = v_parse_expression(parser);

          if(expr_if_false == NULL)
          {
               v_ast_destroy_node(expr_if_true);
               v_ast_destroy_node(condition);
               return NULL;
          }

          expr_if_true = v_ast_new_ternop(parser->ast, condition, expr_if_true, expr_if_false);
     }

     return expr_if_true;
}

VASTNode* v_parse_logical_or(VParser* parser)
{
     VASTNode* left = v_parse_logical_and(parser);

     if(left == NULL)
     {
          return NULL;
     }

     while(v_parser_match_keyword(parser, VKeyword_Or))
     { 
          VASTNode* right = v_parse_logical_and(parser);

          if(right == NULL)
          {
               v_ast_destroy_node(left);
               return NULL;
          }

          left = v_ast_new_binop(parser->ast, VOperator_LogicalOr, left, right);
     }

     return left;
}

VASTNode* v_parse_logical_and(VParser* parser)
{
     VASTNode* left = v_parse_comparison(parser);

     if(left == NULL)
     {
          return NULL;
     }

     while(v_parser_match_keyword(parser, VKeyword_And))
     {
          VASTNode* right = v_parse_comparison(parser);

          if(right == NULL)
          {
               v_ast_destroy_node(left);
               return NULL;
          }

          left = v_ast_new_binop(parser->ast, VOperator_LogicalAnd, left, right);
     }
     return left;
}

VASTNode* v_parse_comparison(VParser* parser)
{
     VASTNode* left = v_parse_bitwise_or(parser);

     if(!left)
     {
          return NULL;
     }

     /* TODO: Implement chained comparisons correctly */

     VToken* token = v_parser_peek(parser);

     if(token == NULL)
     {
          return left;
     }

     VOperator op = VOperator_Unknown;

     if(token->kind == VTokenKind_Operator)
     {
          VOperator current_op =
                         (VOperator)token->type;
          if(current_op == VOperator_ComparatorEquals ||
             current_op == VOperator_ComparatorNotEquals ||
             current_op == VOperator_ComparatorLessThan ||
             current_op == VOperator_ComparatorLessEqualsThan ||
             current_op == VOperator_ComparatorGreaterThan ||
             current_op == VOperator_ComparatorGreaterEqualsThan)
          {
               op = current_op;
               v_parser_advance(parser);
          }
     }
     else if(token->kind == VTokenKind_Keyword)
     {
          VKeyword current_kw = (VKeyword)token->type;

          if(current_kw == VKeyword_Is)
          {
               v_parser_advance(parser);

               if(v_parser_match_keyword(parser, VKeyword_Not))
               {
                    op = VOperator_IdentityIsNot;
               }
               else
               {
                    op = VOperator_IdentityIs;
               }
          }
          else if(current_kw == VKeyword_In)
          {
               op = VOperator_MembershipIn;
               v_parser_advance(parser);
          }
          else if(current_kw == VKeyword_Not)
          {
               v_parser_advance(parser);

               if(!v_parser_consume_keyword(parser,
                                            VKeyword_In,
                                            "Expected \"in\" after \"not\" for \"not in\" operator"))
               {
                    v_ast_destroy_node(left);
                    return NULL;
               }

               op = VOperator_MembershipNotIn;
          }
     }

     if(op != VOperator_Unknown)
     {
          VASTNode* right = v_parse_bitwise_or(parser);

          if(right == NULL)
          {
               v_ast_destroy_node(left);
               return NULL;
          }

          /* TODO: Handle chained comparison here if implementing fully */
          VToken* next_token = v_parser_peek(parser);

          if(next_token != NULL &&
             (next_token->kind == VTokenKind_Operator || next_token->kind == VTokenKind_Keyword))
          {
               VOperator next_op_kind = VOperator_Unknown;

               if(next_token->kind == VTokenKind_Operator)
               {
                    next_op_kind = (VOperator)next_token->type;
               }

               if(next_op_kind == VOperator_ComparatorEquals ||
                  next_op_kind == VOperator_ComparatorNotEquals || /* TODO: other ops */
                  v_parser_check_keyword(parser, VKeyword_Is) ||
                  v_parser_check_keyword(parser, VKeyword_In) ||
                  v_parser_check_keyword(parser, VKeyword_Not))
               {
                    v_parser_set_error(parser, "Chained comparisons not fully supported yet");
                    v_ast_destroy_node(left);
                    v_ast_destroy_node(right);
                    return NULL;
               }
          }

          left = v_ast_new_binop(parser->ast, op, left, right);
     }

     return left;
}

/* Helper for parsing left-associative binary operators at a given precedence level */

typedef VASTNode* (*ParseOperandFunc)(VParser*);

VASTNode* v_parse_binary_op_left_assoc(VParser* parser,
                                       ParseOperandFunc parse_operand,
                                       const VOperator* ops,
                                       int num_ops)
{
     VASTNode* left = parse_operand(parser);

     if(left == NULL)
     {
          return NULL;
     }

     while(true)
     {
          VToken* token = v_parser_peek(parser);

          if(token == NULL || token->kind != VTokenKind_Operator)
          {
               break;
          }

          VOperator current_op = (VOperator)token->type;
          bool match = false;

          for(int i = 0; i < num_ops; ++i)
          {
               if(current_op == ops[i])
               {
                    match = true;
                    break;
               }
          }

          if(!match)
          {
               break;
          }

          v_parser_advance(parser);

          VASTNode* right = parse_operand(parser);

          if(right == NULL)
          {
               v_ast_destroy_node(left);
               return NULL;
          }

          left = v_ast_new_binop(parser->ast, current_op, left, right);
     }

     return left;
}

VASTNode* v_parse_bitwise_or(VParser* parser)
{
     const VOperator ops[] = {VOperator_BitwiseOr};
     return v_parse_binary_op_left_assoc(parser, v_parse_bitwise_xor, ops, 1);
}

VASTNode* v_parse_bitwise_xor(VParser* parser)
{
     const VOperator ops[] = {VOperator_BitwiseXor};
     return v_parse_binary_op_left_assoc(parser, v_parse_bitwise_and, ops, 1);
}

VASTNode* v_parse_bitwise_and(VParser* parser)
{
     const VOperator ops[] = {VOperator_BitwiseAnd};
     return v_parse_binary_op_left_assoc(parser, v_parse_shift, ops, 1);
}

VASTNode* v_parse_shift(VParser* parser)
{
     const VOperator ops[] = {VOperator_BitwiseLShift, VOperator_BitwiseRShift};
     return v_parse_binary_op_left_assoc(parser, v_parse_term, ops, 2);
}

VASTNode* v_parse_term(VParser* parser)
{
     const VOperator ops[] = {VOperator_Addition, VOperator_Subtraction};
     return v_parse_binary_op_left_assoc(parser, v_parse_factor, ops, 2);
}

VASTNode* v_parse_factor(VParser* parser)
{
     const VOperator ops[] = {VOperator_Multiplication,
                              /* TODO: Add @ operator maybe ? */
                              VOperator_Division,
                              VOperator_Modulus,
                              VOperator_FloorDivision};

     return v_parse_binary_op_left_assoc(parser,
                                         v_parse_unary,
                                         ops,
                                         4);
}

VASTNode* v_parse_unary(VParser* parser)
{
     VOperator op = VOperator_Unknown;

     if(v_parser_match_operator(parser, VOperator_Addition))
     {
          op = VOperator_Addition;
     }
     else if(v_parser_match_operator(parser, VOperator_Subtraction))
     {
          op = VOperator_Subtraction;
     }
     else if(v_parser_match_operator(parser, VOperator_BitwiseNot))
     {
          op = VOperator_BitwiseNot;
     }
     else if(v_parser_match_keyword(parser, VKeyword_Not))
     {
          op = VOperator_LogicalNot;
     }

     if(op != VOperator_Unknown)
     {
          VASTNode* operand = v_parse_unary(parser);

          if(operand == NULL)
          {
               return NULL;
          }

          return v_ast_new_unop(parser->ast, op, operand);
     }

     return v_parse_power(parser);
}

VASTNode* v_parse_power(VParser* parser)
{
     VASTNode* left = v_parse_primary(parser);

     if(left == NULL)
     {
          return NULL;
     }

     if(v_parser_match_operator(parser, VOperator_Exponentiation))
     {
          VASTNode* right = v_parse_unary(parser);

          if(right == NULL)
          {
               v_ast_destroy_node(left);
               return NULL;
          }

          left = v_ast_new_binop(parser->ast, VOperator_Exponentiation, left, right);
     }

     return left;
}

VASTNode* v_parse_primary(VParser* parser)
{
     VASTNode* node = v_parse_atom(parser);

     if(node == NULL)
     {
          return NULL;
     }

     while(true)
     {
          if(v_parser_check_delimiter(parser, VDelimiter_LParen))
          {
               node = v_parse_function_call(parser,
                                            node);

               if(node == NULL)
               {
                    return NULL;
               }
          }
          else if(v_parser_check_delimiter(parser, VDelimiter_Dot))
          {
               node = v_parse_attribute_access(parser,
                                               node);

               if(node == NULL)
               {
                    return NULL;
               }
          }
          else if(v_parser_check_delimiter(parser, VDelimiter_LBracket))
          {
               node = v_parse_subscript(parser,
                                        node);

               if(node == NULL)
               {
                    return NULL;
               }
          }
          else
          {
               break;
          }
     }

     return node;
}

VASTNode* v_parse_atom(VParser* parser)
{
     VToken* token = v_parser_peek(parser);

     if(token == NULL)
     {
          v_parser_set_error(parser, "Unexpected end of input looking for atom");
          return NULL;
     }

     if(token->kind == VTokenKind_Literal ||
        token->kind == VTokenKind_Keyword &&
                       (token->type == VKeyword_True || token->type == VKeyword_False ||
                        token->type == VKeyword_None))
     {
          return v_parse_literal(parser);
     }
     else if(token->kind == VTokenKind_Identifier)
     {
          return v_parse_variable(parser);
     }
     else if(v_parser_check_delimiter(parser, VDelimiter_LBracket))
     {
          return v_parse_literal(parser);
     }
     else if(v_parser_check_delimiter(parser, VDelimiter_LBrace))
     {
          return v_parse_literal(parser);
     }
     else if(v_parser_match_delimiter(parser, VDelimiter_LParen))
     {
          VASTNode* node = NULL;
          if(v_parser_match_delimiter(parser, VDelimiter_RParen))
          {
               node = v_ast_new_literal_tuple(parser->ast, vector_new(0, sizeof(VASTNode*)));
          }
          else
          {
               node = v_parse_expression(parser);

               if(node == NULL)
               {
                    return NULL;
               }

               if(v_parser_match_delimiter(parser, VDelimiter_Comma))
               {
                    Vector* elements = vector_new(4, sizeof(VASTNode*));

                    vector_push_back(elements, &node);

                    while(!v_parser_check_delimiter(parser, VDelimiter_RParen))
                    {
                         if(v_parser_check_delimiter(parser, VDelimiter_RParen))
                         {
                              break;
                         }

                         VASTNode* elem = v_parse_expression(parser);

                         if(elem == NULL)
                         {
                              vector_free_with_dtor(elements, v_ast_vector_free_callback);
                              return NULL;
                         }

                         vector_push_back(elements, &elem);

                         if(!v_parser_match_delimiter(parser, VDelimiter_Comma))
                         {
                              if(!v_parser_check_delimiter(parser, VDelimiter_RParen))
                              {
                                   vector_free_with_dtor(elements, v_ast_vector_free_callback);
                                   v_parser_set_error(parser,
                                                      "Expected \",\" or \")\" in tuple display");
                                   return NULL;
                              }

                              break;
                         }
                    }

                    node = v_ast_new_literal_tuple(parser->ast, elements);
               }
               if(!v_parser_consume_delimiter(parser,
                                              VDelimiter_RParen,
                                              "Expected \")\" to close parenthesized expression or tuple"))
               {
                    if(node->type != VASTNodeType_VASTLiteral ||
                       ((VASTLiteral*)node)->lit_type != VType_Tuple)
                    {
                         v_ast_destroy_node(node);
                    }

                    return NULL;
               }
          }

          return node;
     }

     return NULL;
}

VASTNode* v_parse_literal(VParser* parser)
{
     VToken* token = v_parser_peek(parser);
     if(!token)
     {
          v_parser_set_error(parser, "Unexpected end of input looking for literal");
          return NULL;
     }

     if(v_parser_match_keyword(parser, VKeyword_True))
     {
          return v_ast_new_literal_bool(parser->ast, true);
     }
     if(v_parser_match_keyword(parser, VKeyword_False))
     {
          return v_ast_new_literal_bool(parser->ast, false);
     }
     if(v_parser_match_keyword(parser, VKeyword_None))
     {
          return v_ast_new_literal_none(parser->ast);
     }

     if(v_parser_match_delimiter(parser, VDelimiter_LBracket))
     {
          Vector* elements = vector_new(8, sizeof(VASTNode*));

          if(!v_parser_check_delimiter(parser, VDelimiter_RBracket))
          {
               do
               {
                    if(v_parser_check_delimiter(parser, VDelimiter_RBracket))
                    {
                         break;
                    }

                    VASTNode* elem = v_parse_expression(parser);

                    if(elem == NULL)
                    {
                         vector_free_with_dtor(elements, v_ast_vector_free_callback);
                         return NULL;
                    }

                    vector_push_back(elements, &elem);

                    if(!v_parser_match_delimiter(parser, VDelimiter_Comma))
                    {
                         if(!v_parser_check_delimiter(parser, VDelimiter_RBracket))
                         {
                              vector_free_with_dtor(elements, v_ast_vector_free_callback);
                              v_parser_set_error(parser, "Expected \",\" or \"]\" in list display");
                              return NULL;
                         }

                         break;
                    }
               } while(true);
          }

          if(!v_parser_consume_delimiter(parser,
                                         VDelimiter_RBracket,
                                         "Expected \"]\" after list elements"))
          {
               vector_free_with_dtor(elements, v_ast_vector_free_callback);
               return NULL;
          }

          return v_ast_new_literal_list(parser->ast, elements);
     }

     if(v_parser_match_delimiter(parser, VDelimiter_LBrace))
     {
          Vector* keys_or_elements = vector_new(8, sizeof(VASTNode*));
          Vector* values = NULL;

          if(!keys_or_elements)
          {
               v_parser_set_error(parser, "Failed to allocate dict/set keys vector");
               return NULL;
          }

          if(!v_parser_check_delimiter(parser, VDelimiter_RBrace))
          {
               VASTNode* first_item = v_parse_expression(parser);

               if(first_item == NULL)
               {
                    vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);
                    return NULL;
               }

               if(v_parser_match_delimiter(parser, VDelimiter_Colon))
               {
                    values = vector_new(8, sizeof(VASTNode*));

                    VASTNode* first_value = v_parse_expression(parser);

                    if(first_value == NULL)
                    {
                         v_ast_destroy_node(first_item);
                         vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);
                         vector_free(values);
                         return NULL;
                    }

                    vector_push_back(keys_or_elements, &first_item);
                    vector_push_back(values, &first_value);

                    while(v_parser_match_delimiter(parser, VDelimiter_Comma))
                    {
                         if(v_parser_check_delimiter(parser, VDelimiter_RBrace))
                         {
                              break;
                         }

                         VASTNode* key = v_parse_expression(parser);

                         if(key == NULL)
                         {
                              vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);
                              vector_free_with_dtor(values, v_ast_vector_free_callback);
                              return NULL;
                         }

                         if(!v_parser_consume_delimiter(parser,
                                                        VDelimiter_Colon,
                                                        "Expected \":\" after dictionary key"))
                         {
                              v_ast_destroy_node(key);
                              vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);
                              vector_free_with_dtor(values, v_ast_vector_free_callback);
                              return NULL;
                         }

                         VASTNode* val = v_parse_expression(parser);

                         if(val == NULL)
                         {
                              v_ast_destroy_node(key);
                              vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);
                              vector_free_with_dtor(values, v_ast_vector_free_callback);
                              return NULL;
                         }

                         vector_push_back(keys_or_elements, &key);
                         vector_push_back(values, &val);
                    }
               }
               else
               {
                    vector_push_back(keys_or_elements, &first_item);

                    while(v_parser_match_delimiter(parser, VDelimiter_Comma))
                    {
                         if(v_parser_check_delimiter(parser, VDelimiter_RBrace))
                         {
                              break;
                         }

                         VASTNode* elem = v_parse_expression(parser);

                         if(elem == NULL)
                         {
                              vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);
                              return NULL;
                         }
                         vector_push_back(keys_or_elements, &elem);
                    }
               }
          }

          if(!v_parser_consume_delimiter(parser,
                                         VDelimiter_RBrace,
                                         "Expected \"}\" after dictionary or set elements"))
          {
               vector_free_with_dtor(keys_or_elements, v_ast_vector_free_callback);

               if(values != NULL)
               {
                    vector_free_with_dtor(values, v_ast_vector_free_callback);
               }

               return NULL;
          }

          if(values != NULL)
          {
               return v_ast_new_literal_dict(parser->ast, keys_or_elements, values);
          }
          else
          {
               if(vector_size(keys_or_elements) == 0)
               {
                    vector_free(keys_or_elements);

                    return v_ast_new_literal_dict(parser->ast,
                                                  vector_new(0, sizeof(VASTNode*)),
                                                  vector_new(0, sizeof(VASTNode*)));
               }
               else
               {
                    return v_ast_new_literal_set(parser->ast, keys_or_elements);
               }
          }
     }

     if(token->kind == VTokenKind_Literal)
     {
          VLiteral literal_type = (VLiteral)token->type;
          v_parser_advance(parser);

          switch(literal_type)
          {
               case VLiteral_Integer:
                    {
                         char* endptr;
                         long long val = strtoll(token->start,
                                                 &endptr,
                                                 0);

                         if(endptr != token->start + token->length)
                         {
                              v_parser_set_error(parser, "Invalid integer literal format");
                              return NULL;
                         }

                         return v_ast_new_literal_int(parser->ast, (int64_t)val);
                    }
               case VLiteral_Float:
                    {
                         char* endptr;
                         double val = strtod(token->start, &endptr);

                         if(endptr != token->start + token->length)
                         {
                              v_parser_set_error(parser, "Invalid float literal format");
                              return NULL;
                         }

                         return v_ast_new_literal_float(parser->ast, val);
                    }
               case VLiteral_String:
               case VLiteral_UnicodeString:
               case VLiteral_RawString:
                    /* TODO: Handle f-string parsing */
                    /* TODO: Handle literal bytes */
                    {
                         String str_val = string_newf("%.*s", (int)token->length, token->start);

                         /* TODO: Process escape sequences here */
                         /* TODO: Handle different string types (bytes, raw, etc.) maybe with different VType enums ? */

                         return v_ast_new_literal_string(parser->ast, str_val);
                    }
               default:
                    v_parser_set_error(parser,
                                       "Internal error: Unknown VLiteral token type from lexer");
                    return NULL;
          }
     }

     v_parser_set_error(parser,
                        "Expected a literal value (number, string, "
                        "boolean, None, list, dict, set)");

     return NULL;
}

VASTNode* v_parse_variable(VParser* parser)
{
     VToken* token = v_parser_consume_kind(parser, VTokenKind_Identifier, "Expected identifier");

     if(token == NULL)
     {
          return NULL;
     }

     String name = string_newf("%.*s", (int)token->length, token->start);

     return v_ast_new_variable(parser->ast, name, VType_Unknown);
}

Vector* v_parse_argument_list(VParser* parser, Vector** out_kwarg_names, Vector** out_kwarg_values)
{
     Vector* args = NULL;
     Vector* kwarg_names = NULL;
     Vector* kwarg_values = NULL;

     bool parsing_kwargs = false;

     if(!v_parser_check_delimiter(parser, VDelimiter_RParen))
     { 
          do
          {
               if(v_parser_check_delimiter(parser, VDelimiter_RParen))
               {
                    break;
               }

               VASTNode* expr = NULL;
               VToken* possible_kwarg_name = v_parser_peek(parser);

               bool is_kwarg = false;

               if(possible_kwarg_name != NULL && possible_kwarg_name->kind == VTokenKind_Identifier)
               {
                    size_t next_token_idx = parser->current + 1;

                    if(next_token_idx < vector_size(parser->tokens))
                    {
                         VToken* next_token = (VToken*)vector_at(parser->tokens, next_token_idx);

                         if(next_token->kind == VTokenKind_Operator &&
                            next_token->type == VOperator_Assign)
                         {
                              is_kwarg = true;
                         }
                    }
               }

               if(is_kwarg)
               {
                    v_parser_advance(parser);
                    v_parser_advance(parser);

                    String name = string_newf("%.*s",
                                              (int)possible_kwarg_name->length,
                                              possible_kwarg_name->start);

                    VASTNode* value = v_parse_expression(parser);

                    if(value == NULL)
                    {
                         if(args != NULL)
                         {
                              vector_free_with_dtor(args, v_ast_vector_free_callback);
                         }

                         if(kwarg_names != NULL)
                         {
                              vector_free_with_dtor(kwarg_names, v_ast_vector_free_destroy_string);
                              vector_free_with_dtor(kwarg_values, v_ast_vector_free_callback);
                         }

                         string_free(name);

                         return NULL;
                    }

                    if(kwarg_names == NULL)
                    {
                         kwarg_names = vector_new(2, sizeof(String));
                         kwarg_values = vector_new(2, sizeof(VASTNode*));

                    }

                    vector_push_back(kwarg_names, &name);
                    vector_push_back(kwarg_values, &value);

                    parsing_kwargs = true;
               }
               else
               {
                    if(parsing_kwargs)
                    {
                         v_parser_set_error(parser,
                                            "Positional argument cannot follow keyword argument");

                         if(args != NULL)
                         {
                              vector_free_with_dtor(args, v_ast_vector_free_callback);
                         }

                         if(kwarg_names != NULL)
                         {
                              vector_free_with_dtor(kwarg_names, v_ast_vector_free_destroy_string);
                              vector_free_with_dtor(kwarg_values, v_ast_vector_free_callback);
                         }

                         return NULL;
                    }

                    expr = v_parse_expression(parser);

                    if(expr == NULL)
                    {
                         if(args != NULL)
                         {
                              vector_free(args);
                         }

                         return NULL;
                    }

                    if(args == NULL)
                    {
                         args = vector_new(4, sizeof(VASTNode*));
                    }

                    vector_push_back(args, &expr);
               }

               if(!v_parser_match_delimiter(parser, VDelimiter_Comma))
               {
                    if(!v_parser_check_delimiter(parser, VDelimiter_RParen))
                    {
                         v_parser_set_error(parser, "Expected \",\" or \")\" in argument list");

                         if(args != NULL)
                         {
                              vector_free_with_dtor(args, v_ast_vector_free_callback);
                         }

                         if(kwarg_names != NULL)
                         {
                              vector_free_with_dtor(kwarg_names, v_ast_vector_free_destroy_string);
                              vector_free_with_dtor(kwarg_values, v_ast_vector_free_callback);
                         }

                         return NULL;
                    }

                    break;
               }
          } while(true);
     }

     *out_kwarg_names = kwarg_names;
     *out_kwarg_values = kwarg_values;

     return args;
}

VASTNode* v_parse_function_call(VParser* parser, VASTNode* callable_expr)
{
     if(!v_parser_consume_delimiter(parser,
                                    VDelimiter_LParen,
                                    "Expected \"(\" to start function call"))
     {
          v_ast_destroy_node(callable_expr);
          return NULL;
     }

     Vector* args = NULL;
     Vector* kwarg_names = NULL;
     Vector* kwarg_values = NULL;

     args = v_parse_argument_list(parser, &kwarg_names, &kwarg_values);

     if(!v_parser_consume_delimiter(parser, VDelimiter_RParen, "Expected \")\" to end function call"))
     {
          v_ast_destroy_node(callable_expr);
          
          if(args != NULL)
          {
               vector_free_with_dtor(args, v_ast_vector_free_callback);
          }

          if(kwarg_names != NULL)
          {
               vector_free_with_dtor(kwarg_names, v_ast_vector_free_destroy_string);
          }

          if(kwarg_values != NULL)
          {
               vector_free_with_dtor(kwarg_values, v_ast_vector_free_callback);
          }

          return NULL;
     }

     return v_ast_new_fcall(parser->ast, callable_expr, args, kwarg_names, kwarg_values);
}

VASTNode* v_parse_attribute_access(VParser* parser, VASTNode* object_expr)
{
     if(!v_parser_consume_delimiter(parser, VDelimiter_Dot, "Expected \".\" for attribute access"))
     {
          v_ast_destroy_node(object_expr);
          return NULL;
     }

     VToken* attr_token = v_parser_consume_kind(parser,
                                                VTokenKind_Identifier,
                                                "Expected attribute name after \".\"");
     if(attr_token == NULL)
     {
          v_ast_destroy_node(object_expr);
          return NULL;
     }

     String attr_name = string_newf("%.*s", (int)attr_token->length, attr_token->start);

     return v_ast_new_attribute_access(parser->ast, object_expr, attr_name);
}

VASTNode* v_parse_subscript(VParser* parser, VASTNode* object_expr)
{
     if(!v_parser_consume_delimiter(parser, VDelimiter_LBracket, "Expected \"[\" to start subscript"))
     {
          v_ast_destroy_node(object_expr);
          return NULL;
     }

     /* TODO: Add support for tuples: a[1, 2] -> a[(1, 2)] */

     VASTNode* slice_node = v_parse_slice(parser);

     if(slice_node == NULL)
     {
          if(parser->error != NULL)
          {
               v_ast_destroy_node(object_expr);
               return NULL;
          }

          slice_node = v_parse_expression(parser);

          if(slice_node == NULL)
          {
               v_ast_destroy_node(object_expr);

               if(!parser->error)
               {
                    v_parser_set_error(parser, "Expected index expression or slice in subscript");
               }

               return NULL;
          }
     }

     if(!v_parser_consume_delimiter(parser, VDelimiter_RBracket, "Expected \"]\" to end subscript"))
     {
          v_ast_destroy_node(object_expr);
          v_ast_destroy_node(slice_node);
          return NULL;
     }

     return v_ast_new_subscript(parser->ast, object_expr, slice_node);
}

VASTNode* v_parse_slice(VParser* parser)
{
     VASTNode* start = NULL;
     VASTNode* stop = NULL;
     VASTNode* step = NULL;

     if(!v_parser_check_delimiter(parser, VDelimiter_Colon) &&
        !v_parser_check_delimiter(parser, VDelimiter_RBracket))
     {
          start = v_parse_expression(parser);

          if(start == NULL)
          {
               return NULL;
          }
     }

     if(!v_parser_match_delimiter(parser, VDelimiter_Colon))
     {
          if(start != NULL)
          {
               v_ast_destroy_node(start);
          }

          return NULL;
     }

     if(!v_parser_check_delimiter(parser, VDelimiter_Colon) &&
        !v_parser_check_delimiter(parser, VDelimiter_RBracket))
     {
          stop = v_parse_expression(parser);

          if(stop == NULL)
          {
               v_ast_destroy_node(start);
               return NULL;
          }
     }

     if(v_parser_match_delimiter(parser, VDelimiter_Colon))
     {
          if(!v_parser_check_delimiter(parser, VDelimiter_RBracket))
          {
               step = v_parse_expression(parser);

               if(step == NULL)
               {
                    v_ast_destroy_node(start);
                    v_ast_destroy_node(stop);
                    return NULL;
               }
          }
     }

     return v_ast_new_slice(parser->ast, start, stop, step);
}

VType v_parse_type_annotation(VParser* parser)
{
     /* TODO: Handle complex types like List[int], Optional[str], Union[...]... */
     VToken* type_token = v_parser_peek(parser);

     if(type_token && type_token->kind == VTokenKind_Identifier)
     {
          v_parser_advance(parser);

          VType type = v_string_to_type(type_token->start);

          if(v_parser_check_delimiter(parser, VDelimiter_LBracket))
          {
               v_parser_set_error(parser,
                                  "Generic types (e.g., list[int]) not fully "
                                  "supported in type annotations yet");

               return VType_Unknown;
          }

          if(type == VType_Unknown)
          {
               return VType_Object;
          }

          return type;
     }
     else
     {
          if(type_token->kind == VTokenKind_Literal &&
             ((VLiteral)type_token->type) == VLiteral_String)
          {
               /* TODO: Handle forward reference strings */
               v_parser_set_error(parser,
                                  "String literal type hints (forward "
                                  "references) not supported yet");
               return VType_Unknown;
          }
     }

     v_parser_set_error(parser, "Expected type name (identifier) after \":\" or \"->\"");

     return VType_Unknown;
}

Vector* v_parse_parameter_list(VParser* parser)
{
     Vector* params = vector_new(4, sizeof(VASTNode*));

     if(!v_parser_consume_delimiter(parser,
                                    VDelimiter_LParen,
                                    "Expected \"(\" to start parameter list"))
     {
          vector_free(params);
          return NULL;
     }

     bool parsing_defaults = false;

     if(!v_parser_check_delimiter(parser, VDelimiter_RParen))
     {
          do
          {
               if(v_parser_check_delimiter(parser, VDelimiter_RParen))
               {
                    break;
               }

               /* TODO: Handle *args, **kwargs, positional-only /, keyword-only * markers */

               VToken* param_token = v_parser_consume_kind(parser,
                                                           VTokenKind_Identifier,
                                                           "Expected parameter name");
               if(param_token == NULL)
               {
                    vector_free_with_dtor(params, v_ast_vector_free_callback);
                    return NULL;
               }

               String param_name = string_newf("%.*s",
                                               (int)param_token->length,
                                               param_token->start);

               VType param_type = VType_Unknown;
               VASTNode* default_value = NULL;

               if(v_parser_match_delimiter(parser, VDelimiter_Colon))
               {
                    param_type = v_parse_type_annotation(parser);

                    if(parser->error != NULL)
                    {
                         string_free(param_name);
                         vector_free_with_dtor(params, v_ast_vector_free_callback);
                         return NULL;
                    }
               }

               if(v_parser_match_operator(parser, VOperator_Assign))
               {
                    parsing_defaults = true;
                    default_value = v_parse_expression(parser);

                    if(default_value == NULL)
                    { 
                         string_free(param_name);
                         vector_free_with_dtor(params, v_ast_vector_free_callback);

                         return NULL;
                    }
               }
               else
               {
                    if(parsing_defaults)
                    {
                         string_free(param_name);
                         vector_free_with_dtor(params, v_ast_vector_free_callback);
                         v_parser_set_error(parser, "Non-default argument follows default argument");
                         return NULL;
                    }
               }

               VASTNode* param_node = v_ast_new_argument(parser->ast,
                                                         param_name,
                                                         param_type,
                                                         default_value);

               vector_push_back(params,
                                &param_node);

               if(!v_parser_match_delimiter(parser, VDelimiter_Comma))
               {
                    if(!v_parser_check_delimiter(parser, VDelimiter_RParen))
                    {
                         v_parser_set_error(parser, "Expected \",\" or \")\" in parameter list");
                         vector_free_with_dtor(params, v_ast_vector_free_callback);
                         return NULL;
                    }

                    break;
               }
          } while(true);
     }

     if(!v_parser_consume_delimiter(parser,
                                    VDelimiter_RParen,
                                    "Expected \")\" to end parameter list"))
     {
          vector_free_with_dtor(params, v_ast_vector_free_callback);
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
          case VASTNodeType_VASTParameter:
               v_ast_destroy_argument(node);
               break;
          case VASTNodeType_VASTLiteral:
               v_ast_destroy_literal(node);
               break;
          case VASTNodeType_VASTFCall:
               v_ast_destroy_fcall(node);
               break;
          case VASTNodeType_VASTAttributeAccess:
               v_ast_destroy_attribute_access(node);
               break;
          case VASTNodeType_VASTSubscript:
               v_ast_destroy_subscript(node);
               break;
          case VASTNodeType_VASTSlice:
               v_ast_destroy_slice(node);
               break;
          case VASTNodeType_VASTPass:
          case VASTNodeType_VASTBreak:
          case VASTNodeType_VASTContinue:
               break;
          default:
               break;
     }
}

void v_ast_destroy(VAST* ast)
{
     if(ast != NULL)
     {
          if(ast->error != NULL)
          {
               string_free(ast->error);
          }

          v_ast_destroy_node(ast->root);

          arena_destroy(&ast->data);

          free(ast);

          ast = NULL;
     }
}

/*******************************/
/* VASTNodes functions */

VASTNode* v_ast_new_source(VAST* ast, Vector* decls)
{
     VASTSource source = {{VASTNodeType_VASTSource}, decls};

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
                    {VASTNodeType_VASTClass},
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
                    {VASTNodeType_VASTFunction},
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

     v_ast_destroy_node((VASTNode*)func->body);
     vector_free_with_dtor(func->params, v_ast_vector_free_callback);
     vector_free_with_dtor(func->decorators, v_ast_vector_free_callback);
}

VASTNode* v_ast_new_body(VAST* ast, Vector* stmts)
{
     VASTBody body = {
                    {VASTNodeType_VASTBody},
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

VASTNode* v_ast_new_for(VAST* ast, bool is_while, VASTNode* target, VASTNode* cond, VASTBody* body)
{
     VASTFor for_loop = {
                    {VASTNodeType_VASTFor},
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
     v_ast_destroy_node((VASTNode*)f->body);
}

VASTNode* v_ast_new_if(VAST* ast, VASTNode* condition, VASTBody* body, VASTNode* else_node)
{
     VASTIf if_ = {
                    {VASTNodeType_VASTIf},
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
     v_ast_destroy_node((VASTNode*)i->body);
     v_ast_destroy_node(i->else_node);
}

VASTNode* v_ast_new_return(VAST* ast, VASTNode* value)
{
     VASTReturn ret = {
                    {VASTNodeType_VASTReturn},
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
                    {VASTNodeType_VASTAssignment},
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

VASTNode* v_ast_new_unop(VAST* ast, const VOperator op, VASTNode* operand)
{
     VASTUnOp unop = {
                    {VASTNodeType_VASTUnOp},
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

VASTNode* v_ast_new_binop(VAST* ast, const VOperator op, VASTNode* left, VASTNode* right)
{
     VASTBinOp binop = {
                    {VASTNodeType_VASTBinOp},
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

VASTNode* v_ast_new_ternop(VAST* ast, VASTNode* condition, VASTNode* if_expr, VASTNode* else_expr)
{
     VASTTernOp ternop = {
                    {VASTNodeType_VASTTernOp},
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

VASTNode* v_ast_new_decorator(VAST* ast, const String name)
{
     VASTDecorator decorator = {
                    {VASTNodeType_VASTDecorator},
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
                              const VType type,
                              VASTNode* initial_value)
{
     VASTAttribute attribute = {
                    {VASTNodeType_VASTAttribute},
                    name,
                    type,
                    initial_value,
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

VASTNode* v_ast_new_variable(VAST* ast, const String name, const VType type)
{
     VASTVariable var = {
                    {VASTNodeType_VASTVariable},
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
     VASTParameter arg = {
                    {VASTNodeType_VASTParameter},
                    name,
                    type,
                    default_value,
     };

     void* addr = arena_push(&ast->data, &arg, sizeof(VASTParameter));

     return (VASTNode*)addr;
}

void v_ast_destroy_argument(VASTNode* argument)
{
     VASTParameter* arg = VAST_CAST(VASTParameter, argument);
     VASSERT_TYPE(VASTParameter, arg);

     string_free(arg->name);

     v_ast_destroy_node(arg->default_value);
}

VASTNode* v_ast_new_literal_int(VAST* ast, const int64_t value)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_Int;
     lit.value.int_val = value;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_float(VAST* ast, const double value)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_Float;
     lit.value.float_val = value;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_string(VAST* ast, const String value)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_String;
     lit.value.str_val = value;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_bool(VAST* ast, const bool value)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_Bool;
     lit.value.bool_val = value;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_none(VAST* ast)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_None;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_list(VAST* ast, Vector* elements)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_List;
     lit.value.list_val.elements = elements;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_dict(VAST* ast, Vector* keys, Vector* values)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_Dict;
     lit.value.dict_val.keys = keys;
     lit.value.dict_val.values = values;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_tuple(VAST* ast, Vector* elements)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
     lit.lit_type = VType_Tuple;
     lit.value.tuple_val.elements = elements;

     void* addr = arena_push(&ast->data, &lit, sizeof(VASTLiteral));

     return (VASTNode*)addr;
}

VASTNode* v_ast_new_literal_set(VAST* ast, Vector* elements)
{
     VASTLiteral lit = {{VASTNodeType_VASTLiteral}};
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
                          VASTNode* callable,
                          Vector* args,
                          Vector* kwarg_names,
                          Vector* kwarg_values)
{
     VASTFCall fcall = {{VASTNodeType_VASTFCall},
                        callable,
                        args,
                        {
                                       kwarg_names,
                                       kwarg_values,
                        }};

     void* addr = arena_push(&ast->data, &fcall, sizeof(VASTFCall));

     return (VASTNode*)addr;
}

void v_ast_destroy_fcall(VASTNode* fcall)
{
     VASTFCall* f = VAST_CAST(VASTFCall, fcall);
     VASSERT_TYPE(VASTFCall, f);

     v_ast_destroy_node(f->callable);

     vector_free_with_dtor(f->args, v_ast_vector_free_callback);
     vector_free_with_dtor(f->kwargs.names, v_ast_vector_free_destroy_string);
     vector_free_with_dtor(f->kwargs.values, v_ast_vector_free_callback);
}

VASTNode* v_ast_new_attribute_access(VAST* ast, VASTNode* object, const String attribute_name)
{
     VASTAttributeAccess access = {
                    {VASTNodeType_VASTAttributeAccess},
                    object,
                    attribute_name,
     };

     void* addr = arena_push(&ast->data, &access, sizeof(VASTAttributeAccess));

     return (VASTNode*)addr;
}

void v_ast_destroy_attribute_access(VASTNode* attributeaccess)
{
     VASTAttributeAccess* access = VAST_CAST(VASTAttributeAccess, attributeaccess);
     VASSERT_TYPE(VASTAttributeAccess, access);

     v_ast_destroy_node(access->object);

     if(access->attribute_name != NULL)
     {
          string_free(access->attribute_name);
     }
}

VASTNode* v_ast_new_subscript(VAST* ast, VASTNode* value, VASTNode* slice)
{
     VASTSubscript sub = {
                    {VASTNodeType_VASTSubscript},
                    value,
                    slice,
     };

     void* addr = arena_push(&ast->data, &sub, sizeof(VASTSubscript));

     return (VASTNode*)addr;
}

void v_ast_destroy_subscript(VASTNode* subscript)
{
     VASTSubscript* sub = VAST_CAST(VASTSubscript, subscript);
     VASSERT_TYPE(VASTSubscript, sub);

     v_ast_destroy_node(sub->value);
     v_ast_destroy_node(sub->slice);
}

VASTNode* v_ast_new_slice(VAST* ast, VASTNode* start, VASTNode* stop, VASTNode* step)
{
     VASTSlice slice = {
                    {VASTNodeType_VASTSlice},
                    start,
                    stop,
                    step,
     };

     void* addr = arena_push(&ast->data, &slice, sizeof(VASTSlice));

     return (VASTNode*)addr;
}

void v_ast_destroy_slice(VASTNode* slice)
{
     VASTSlice* slc = VAST_CAST(VASTSlice, slice);
     VASSERT_TYPE(VASTSlice, slice);

     v_ast_destroy_node(slc->start);
     v_ast_destroy_node(slc->stop);
     v_ast_destroy_node(slc->step);
}

VASTNode* v_ast_new_pass(VAST* ast)
{
     VASTPass new_pass = {{VASTNodeType_VASTPass}};

     void* addr = arena_push(&ast->data, &new_pass, sizeof(VASTPass));

     return (VASTNode*)addr;
}

void v_ast_destroy_pass(VASTNode* pass_)
{
     return;
}

VASTNode* v_ast_new_break(VAST* ast)
{
     VASTBreak new_break = {{VASTNodeType_VASTBreak}};

     void* addr = arena_push(&ast->data, &new_break, sizeof(VASTBreak));

     return (VASTNode*)addr;
}

void v_ast_destroy_break(VASTNode* break_)
{
     return;
}

VASTNode* v_ast_new_continue(VAST* ast)
{
     VASTContinue new_continue = {{VASTNodeType_VASTContinue}};

     void* addr = arena_push(&ast->data, &new_continue, sizeof(VASTContinue));

     return (VASTNode*)addr;
}

void v_ast_destroy_continue(VASTNode* continue_)
{
     return;
}