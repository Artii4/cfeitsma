#include "array.h"
#include "ast.h"
#include <stdbool.h>
#include <stdio.h>

struct interpreter {
	FILE *output;
	struct array *variables;
	char *error;
	int last_var;
};

struct interpreter interpreter_create(FILE *output);
void interpreter_destroy(struct interpreter *i);
void interpreter_interpret(struct interpreter *i, struct ast *ast);
