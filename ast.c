#define _GNU_SOURCE

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

array_create_declare(struct ast *, ast_p)
array_destroy_declare(struct ast *, ast_p)
array_push_declare(struct ast *, ast_p)

static const char *const ast_type_to_string[] = {
	[ast_plus] = "+",
	[ast_minus] = "-",
	[ast_star] = "*",
	[ast_slash] = "/",
	[ast_number] = "number",
	[ast_name] = "name",
	[ast_het] = "het",
	[ast_print] = "print",
	[ast_assign] = "=",
	[ast_program] = "program"
};

struct ast *ast_create(enum ast_type type)
{
	struct ast *ast = malloc(sizeof(struct ast));
	if (ast == NULL)
		return NULL;

	*ast = (struct ast) {
		.type = type,
		.children = array_create_ast_p()
	};

	return ast;
}

struct ast *ast_create_name(char *name_value)
{
	struct ast *ast = malloc(sizeof(struct ast));
	if (ast == NULL)
		return NULL;

	*ast = (struct ast) {
		.type = ast_name,
		.name_value = name_value,
		.children = array_create_ast_p()
	};

	return ast;
}

struct ast *ast_create_number(double number_value)
{
	struct ast *ast = malloc(sizeof(struct ast));
	if (ast == NULL)
		return NULL;

	*ast = (struct ast) {
		.type = ast_number,
		.number_value = number_value,
		.children = array_create_ast_p()
	};

	return ast;
}

void ast_destroy(struct ast *a)
{
	for (size_t i = 0; i < a->children.nelts; i++)
		ast_destroy(a->children.elts[i]);

	array_destroy_ast_p(a->children);

	if (a->type == ast_name)
		free(a->name_value);

	free(a);
}

static char *ast_single_to_string(struct ast *a)
{
	char *s;

	if (a->type == ast_number)
		asprintf(&s, "%.5f", a->number_value);
	else if (a->type == ast_name)
		asprintf(&s, "\"%s\"", a->name_value);
	else
		asprintf(&s, "%s", ast_type_to_string[a->type]);

	return s;
}

void ast_add_child(struct ast *a, struct ast *child)
{
	*array_push_ast_p(&a->children) = child;
}

char *ast_to_string(struct ast *a)
{
	char *single = ast_single_to_string(a);
	if (a->children.nelts == 0)
		return single;

	char *children_str = strdup("");
	for (size_t i = 0; i < a->children.nelts; i++) {
		char *old = children_str;
		char *new = ast_to_string(a->children.elts[i]);

		if (i == a->children.nelts - 1)
			asprintf(&children_str, "%s%s", children_str, new);
		else
			asprintf(&children_str, "%s%s, ", children_str, new);

		free(old);
		free(new);
	}

	char *s;
	asprintf(&s, "%s (%s)", single, children_str);
	free(single);
	free(children_str);
	return s;
}
