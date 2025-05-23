/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/symtable.h"

#include "libromano/filesystem.h"
#include "libromano/logger.h"
#include "libromano/string.h"

int main(int argc, char** argv)
{
     VENOM_ATEXIT_REGISTER(logger_release, false);

     logger_init();

     logger_log_info("Starting symtable test");

     String file_path = string_newf("%s/test1.py", TESTS_DATA_DIR);

     FileContent content;

     if(!fs_file_content_new((char*)file_path, &content))
     {
          logger_log_error("Cannot content of file: %s", file_path);
          string_free(file_path);
          return 1;
     }

     string_free(file_path);

     printf("%.*s\n", (int)content.content_length, content.content);

     Vector* tokens = vector_new(128, sizeof(VToken));

     if(!v_lexer_lex(content.content, tokens))
     {
          fs_file_content_free(&content);
          vector_free(tokens);
          logger_log_error("Error caught while lexing");
          return 1;
     }

     VAST* ast = v_ast_new();

     if(!v_ast_from_tokens(ast, tokens) || ast->error != NULL)
     {
          logger_log_error("Error caught while building AST: %s", ast->error);
          v_ast_destroy(ast);
          fs_file_content_free(&content);
          vector_free(tokens);
          return 1;
     }

     VSymTable symtable;
     v_symtable_new(&symtable);

     v_symtable_collect(&symtable, ast);
     v_symtable_resolve(&symtable, ast);

     v_symtable_destroy(&symtable);
     v_ast_destroy(ast);
     fs_file_content_free(&content);
     vector_free(tokens);

     logger_log_info("Finished symtable test");

     return 0;
}
