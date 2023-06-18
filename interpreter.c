#define _GNU_SOURCE

#include "interpreter.h"
#include "ast.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct variable {
	char *name;
	double value;
};

static void interpreter_statement(struct interpreter *self, struct ast *ast);
static void interpreter_assign(struct interpreter *self, struct ast *ast);
static void interpreter_print(struct interpreter *self, struct ast *ast);
static double interpreter_expression(struct interpreter *self, struct ast *ast);
static double interpreter_name(struct interpreter *self, struct ast *ast);

struct interpreter interpreter_create(FILE *output)
{
	return (struct interpreter) {
		.output = output,
		.variables = array_create(sizeof(struct variable)),
		.error = NULL,
		.last_var = -1
	};
}

void interpreter_destroy(struct interpreter *self)
{
	for (size_t i = 0; i < self->variables->nelts; i++)
		free(((struct variable *)self->variables->elts)[i].name);

	array_destroy(self->variables);
	free(self->error);
}

void interpreter_interpret(struct interpreter *self, struct ast *ast)
{
	free(self->error);
	self->error = NULL;
	for (size_t i = 0; i < ast->children->nelts; i++) {
		struct ast **c = ast->children->elts;
		interpreter_statement(self, c[i]);
		if (self->error)
			return;
	}
}

static void interpreter_statement(struct interpreter *i, struct ast *ast)
{
	if (ast->type == ast_print)
		interpreter_print(i, ast);
	else if (ast->type == ast_assign)
		interpreter_assign(i, ast);
	else
		assert(false);
}

static void interpreter_print(struct interpreter *self, struct ast *ast)
{
	struct ast **c = ast->children->elts;
	double n = interpreter_expression(self, c[0]);
	if (self->error)
		return;

	fprintf(self->output, "%f\n", n);
}

static void interpreter_assign(struct interpreter *self, struct ast *ast)
{
	struct ast **c = ast->children->elts;
	char *name;

	if (c[0]->type == ast_name) {
		name = c[0]->name_value;
	} else if (c[0]->type == ast_het) {
		if (self->last_var < 0) {
			self->error = strdup("\"het\" is invalid here");
			return;
		}
		struct variable *vars = self->variables->elts;
		name = vars[self->last_var].name;
	}

	double value = interpreter_expression(self, c[1]);

	if (self->error)
		return;

	struct variable *vars = self->variables->elts;

	for (size_t i = 0; i < self->variables->nelts; i++) {
		if (strcmp(vars[i].name, name) == 0) {
			vars[i].value = value;
			self->last_var = i;
		}
	}

	self->last_var = self->variables->nelts;
	*(struct variable *)array_push(self->variables) =
	    (struct variable) { strdup(name), value };
}

static double interpreter_expression(struct interpreter *self, struct ast *ast)
{
	if (ast->type == ast_name) {
		return interpreter_name(self, ast);
	} else if (ast->type == ast_number) {
		return ast->number_value;
	} else if (ast->type == ast_het) {
		if (self->last_var < 0) {
			self->error = strdup("\"het\" is invalid here");
			return 0;
		}
		struct variable *vars = self->variables->elts;
		return vars[self->last_var].value;
	}

	struct ast **children = ast->children->elts;
	double left = interpreter_expression(self, children[0]);
	if (self->error)
		return 0;
	double right = interpreter_expression(self, children[1]);
	if (self->error)
		return 0;

	switch (ast->type) {
	case ast_plus: return left + right;
	case ast_minus: return left - right;
	case ast_star: return left * right;
	case ast_slash: return left / right;
	default: assert(false);
	}
}

static double interpreter_name(struct interpreter *self, struct ast *ast)
{
	struct variable *vars = self->variables->elts;
	for (size_t i = 0; i < self->variables->nelts; i++) {
		if (strcmp(vars[i].name, ast->name_value) == 0)
			return vars[i].value;
	}

	asprintf(&self->error, "variable named \"%s\" doesn't exist",
		 ast->name_value);
	return -1;
}
