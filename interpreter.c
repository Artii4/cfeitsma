#define _GNU_SOURCE

#include "interpreter.h"
#include "ast.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

array_create_declare(struct variable, variable)
array_destroy_declare(struct variable, variable)
array_push_declare(struct variable, variable)

static void interpreter_statement(struct interpreter *self, struct ast *ast);
static void interpreter_assign(struct interpreter *self, struct ast *ast);
static void interpreter_print(struct interpreter *self, struct ast *ast);
static double interpreter_expression(struct interpreter *self, struct ast *ast);
static double interpreter_name(struct interpreter *self, struct ast *ast);

struct interpreter interpreter_create(FILE *output)
{
	return (struct interpreter) {
		.output = output,
		.variables = array_create_variable(),
		.error = NULL,
		.last_var = -1
	};
}

void interpreter_destroy(struct interpreter *self)
{
	for (size_t i = 0; i < self->variables.nelts; i++)
		free(self->variables.elts[i].name);

	array_destroy_variable(self->variables);
	free(self->error);
}

void interpreter_interpret(struct interpreter *self, struct ast *ast)
{
	free(self->error);
	self->error = NULL;
	for (size_t i = 0; i < ast->children.nelts; i++) {
		interpreter_statement(self, ast->children.elts[i]);
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
	double n = interpreter_expression(self, ast->children.elts[0]);
	if (self->error)
		return;

	fprintf(self->output, "%f\n", n);
}

static void interpreter_assign(struct interpreter *self, struct ast *ast)
{
	char *name;

	if (ast->children.elts[0]->type == ast_name) {
		name = ast->children.elts[0]->name_value;
	} else if (ast->children.elts[0]->type == ast_het) {
		if (self->last_var < 0) {
			self->error = strdup("\"het\" is invalid here");
			return;
		}
		name = self->variables.elts[self->last_var].name;
	} else {
		assert(false);
	}

	double value = interpreter_expression(self, ast->children.elts[1]);

	if (self->error)
		return;

	for (size_t i = 0; i < self->variables.nelts; i++) {
		if (strcmp(self->variables.elts[i].name, name) == 0) {
			self->variables.elts[i].value = value;
			self->last_var = i;
		}
	}

	self->last_var = self->variables.nelts;
	*array_push_variable(&self->variables) = (struct variable) { strdup(name), value };
}

static double interpreter_expression(struct interpreter *self, struct ast *ast)
{
	switch(ast->type) {
	case ast_name: return interpreter_name(self, ast);
	case ast_number: return ast->number_value;
	case ast_het:
		if (self->last_var >= 0)
			return self->variables.elts[self->last_var].value;

		self->error = strdup("\"het\" is invalid here");
		return 0;
	}

	double left = interpreter_expression(self, ast->children.elts[0]);
	if (self->error)
		return 0;
	double right = interpreter_expression(self, ast->children.elts[1]);
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
	for (size_t i = 0; i < self->variables.nelts; i++) {
		if (strcmp(self->variables.elts[i].name, ast->name_value) == 0)
			return self->variables.elts[i].value;
	}

	asprintf(&self->error, "variable named \"%s\" doesn't exist", ast->name_value);
	return -1;
}
