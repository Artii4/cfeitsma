#pragma once

#include "ast.h"
#include "lexer.h"

struct parser {
	struct lexer input;
	struct token lookahead;
};

struct parser_result {
	struct ast *ast;
	char *error;
};

struct parser parser_create(char *input);
void parser_destroy(struct parser *p);
struct parser_result parser_parse(struct parser *p);
