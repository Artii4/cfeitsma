#include "array.h"
#include "ast.h"
#include <stdbool.h>
#include <stdio.h>

struct variable {
	char *name;
	double value;
};

array_all_declare(struct variable, variable)

struct interpreter {
	FILE *output;
	struct array_variable variables;
	char *error;
	int last_var;
};

struct interpreter interpreter_create(FILE *output);
void interpreter_destroy(struct interpreter *i);
void interpreter_interpret(struct interpreter *i, struct ast *ast);
