#pragma once

#include "token.h"

struct lexer {
	char *input;
	int index;
};

struct lexer lexer_create(char *input);
struct token lexer_next_token(struct lexer *l);
