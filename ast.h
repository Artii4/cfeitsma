#pragma once

#include "array.h"

enum ast_type {
	ast_plus,
	ast_minus,
	ast_star,
	ast_slash,

	ast_number,
	ast_name,
	ast_het,

	ast_print,
	ast_assign,

	ast_program
};

struct ast {
	enum ast_type type;
	struct array *children;
	union {
		double number_value;
		char *name_value;
	};
};

struct ast *ast_create(enum ast_type type);
struct ast *ast_create_name(char *name_value);
struct ast *ast_create_number(double number_value);
void ast_destroy(struct ast *a);
void ast_add_child(struct ast *a, struct ast *child);
char *ast_to_string(struct ast *a);
