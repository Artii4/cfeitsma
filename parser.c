#define _GNU_SOURCE

#include "ast.h"
#include "parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct infix_bp {
	int left;
	int right;
};

static struct parser_result parser_result_create(struct ast *a);
static struct parser_result parser_result_create_error(char *e);
static struct parser_result parser_error(struct parser *self, char const *want);
static struct parser_result parser_error_lookahead(struct parser *self);
static struct parser_result parser_error_type(struct parser *self, enum token_type t);

static void parser_consume(struct parser *self);
static bool parser_expect(struct parser *self, enum token_type type);

static struct parser_result parser_program(struct parser *self);
static struct parser_result parser_statement(struct parser *self);
static struct parser_result parser_assignment(struct parser *self);
static struct parser_result parser_print(struct parser *self);
static struct parser_result parser_expression(struct parser *self, int min_bp);
static struct infix_bp get_infix_bp(enum token_type t);

struct parser parser_create(char *input)
{
	struct parser p;
	p.input = lexer_create(input);
	p.lookahead = lexer_next_token(&p.input);
	return p;
}

void parser_destroy(struct parser *self)
{
	token_destroy(&self->lookahead);
}

static struct parser_result parser_result_create(struct ast *a)
{
	return (struct parser_result) {
		.ast = a
	};
}

static struct parser_result parser_result_create_error(char *e)
{
	return (struct parser_result) {
		.error = e
	};
}

static struct parser_result parser_error_lookahead(struct parser *self)
{
	return parser_result_create_error(strdup(self->lookahead.error));
}

static struct parser_result parser_error(struct parser *self, char const *want)
{
	if (self->lookahead.error)
		return parser_error_lookahead(self);

	char *error;
	char *tokenstring = token_to_string(self->lookahead);
	asprintf(&error, "Want %s, got %s.", want, tokenstring);
	free(tokenstring);
	return parser_result_create_error(error);
}

static struct parser_result parser_error_type(struct parser *self,
					      enum token_type t)
{
	return parser_error(self, token_type_to_string(t));
}

static void parser_consume(struct parser *self)
{
	token_destroy(&self->lookahead);
	self->lookahead = lexer_next_token(&self->input);
}

static bool parser_expect(struct parser *self, enum token_type t)
{
	if (self->lookahead.error)
		return false;
	if (self->lookahead.type != t)
		return false;

	parser_consume(self);
	return true;
}

static bool parser_expect_peek(struct parser *self, enum token_type t)
{
	return self->lookahead.error == NULL && self->lookahead.type == t;
}

struct parser_result parser_parse(struct parser *self)
{
	struct parser_result result = parser_program(self);
	if (result.error)
		return result;

	if (!parser_expect(self, token_end)) {
		free(result.ast);
		return parser_error_type(self, token_end);
	}

	return result;
}

static struct parser_result parser_program(struct parser *self)
{
	struct ast *a = ast_create(ast_program);

	while (self->lookahead.type != token_end) {
		struct parser_result stmt = parser_statement(self);
		if (stmt.error) {
			ast_destroy(a);
			return stmt;
		}

		ast_add_child(a, stmt.ast);
	}

	return parser_result_create(a);
}

static struct parser_result parser_statement(struct parser *self)
{
	struct parser_result result;

	if (self->lookahead.type == token_laat)
		result = parser_assignment(self);
	else if (self->lookahead.type == token_print)
		result = parser_print(self);
	else
		return parser_error(self, "assignment or print");

	if (result.error)
		return result;

	if (!parser_expect(self, token_semicolon)) {
		ast_destroy(result.ast);
		return parser_error_type(self, token_semicolon);
	}

	return result;
}

static struct parser_result parser_assignment(struct parser *self)
{
	if (!parser_expect(self, token_laat))
		return parser_error_type(self, token_laat);

	if (self->lookahead.error || (self->lookahead.type != token_name
				   && self->lookahead.type != token_het)) {
		return parser_error(self, "name or het");
	}

	char *assignee = strdup(self->lookahead.name_value);
	parser_consume(self);

	struct parser_result expr = parser_expression(self, 0);
	if (expr.error) {
		free(assignee);
		return expr;
	}

	if (!parser_expect(self, token_zijn)) {
		free(assignee);
		ast_destroy(expr.ast);
		return parser_error_type(self, token_zijn);
	}

	struct ast *ast = ast_create(ast_assign);
	ast_add_child(ast, ast_create_name(assignee));
	ast_add_child(ast, expr.ast);

	return parser_result_create(ast);

}

static struct parser_result parser_print(struct parser *self)
{
	if (!parser_expect(self, token_print))
		return parser_error_type(self, token_print);

	struct parser_result expr = parser_expression(self, 0);
	if (expr.error)
		return expr;

	if (!parser_expect(self, token_uit))
		return parser_error_type(self, token_uit);

	struct ast *ast = ast_create(ast_print);
	ast_add_child(ast, expr.ast);

	return parser_result_create(ast);
}

static struct infix_bp get_infix_bp(enum token_type t)
{
	switch (t) {
	case token_plus:
	case token_minus:
		return (struct infix_bp) { 1, 2 };
	case token_star:
	case token_slash:
		return (struct infix_bp) { 3, 4 };
	default:
		return (struct infix_bp) { -1, -1 };
	}
}

static struct parser_result parser_expression(struct parser *self,
						    int min_bp)
{
	struct ast *lhs;

	if (self->lookahead.error)
		return parser_error_lookahead(self);

	switch (self->lookahead.type) {
	case token_number:
		lhs = ast_create_number(self->lookahead.number_value);
		break;
	case token_name:
		lhs = ast_create_name(strdup(self->lookahead.name_value));
		break;
	case token_het:
		lhs = ast_create(ast_het);
		break;
	case token_lparen: {
		parser_consume(self);
		struct parser_result l = parser_expression(self, 0);
		if (l.error)
			return l;

		lhs = l.ast;

		if (!parser_expect_peek(self, token_rparen))
			return parser_error_type(self, token_rparen);

		break;
	}
	default:
		return parser_error(self, "number, name, het, or (");
	}

	parser_consume(self);

	for (;;) {
		struct infix_bp bp = get_infix_bp(self->lookahead.type);

		if (bp.left < min_bp)
			break;

		struct ast *op;
		if (self->lookahead.type == token_plus)
			op = ast_create(ast_plus);
		else if (self->lookahead.type == token_minus)
			op = ast_create(ast_minus);
		else if (self->lookahead.type == token_star)
			op = ast_create(ast_star);
		else if (self->lookahead.type == token_slash)
			op = ast_create(ast_slash);

		parser_consume(self);

		struct parser_result rhs = parser_expression(self, bp.right);
		if (rhs.error) {
			ast_destroy(lhs);
			return rhs;
		}

		ast_add_child(op, lhs);
		ast_add_child(op, rhs.ast);

		lhs = op;
	}

	return parser_result_create(lhs);
}
