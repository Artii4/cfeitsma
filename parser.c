#define _GNU_SOURCE

#include "ast.h"
#include "parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct parser_result parser_result_create(struct ast *a);
static struct parser_result parser_result_create_error(char *e);

static struct parser_result parser_error(struct parser *p, char const *want);
static struct parser_result parser_error_lookahead(struct parser *p);
static struct parser_result parser_error_type(struct parser *p,
					      enum token_type t);

static void parser_consume(struct parser *p);
static bool parser_expect(struct parser *p, enum token_type type);

static struct parser_result parser_program(struct parser *p);
static struct parser_result parser_statement(struct parser *p);
static struct parser_result parser_assignment(struct parser *p);
static struct parser_result parser_print(struct parser *p);
static struct parser_result parser_expression(struct parser *p);
static struct parser_result parser_mul(struct parser *p);
static struct parser_result parser_value(struct parser *p);

struct parser parser_create(char *input)
{
	struct parser p;
	p.input = lexer_create(input);
	p.lookahead = lexer_next_token(&p.input);
	return p;
}

void parser_destroy(struct parser *p)
{
	token_destroy(&p->lookahead);
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

static struct parser_result parser_error_lookahead(struct parser *p)
{
	return parser_result_create_error(strdup(p->lookahead.error));
}

static struct parser_result parser_error(struct parser *p, char const *want)
{
	if (p->lookahead.error)
		return parser_error_lookahead(p);

	char *error;
	char *tokenstring = token_to_string(p->lookahead);
	asprintf(&error, "Want %s, got %s.", want, tokenstring);
	free(tokenstring);
	return parser_result_create_error(error);
}

static struct parser_result parser_error_type(struct parser *p,
					      enum token_type t)
{
	return parser_error(p, token_type_to_string(t));
}

static void parser_consume(struct parser *p)
{
	token_destroy(&p->lookahead);
	p->lookahead = lexer_next_token(&p->input);
}

static bool parser_expect(struct parser *p, enum token_type t)
{
	if (p->lookahead.error)
		return false;
	if (p->lookahead.type != t)
		return false;

	parser_consume(p);
	return true;
}

struct parser_result parser_parse(struct parser *p)
{
	struct parser_result result = parser_program(p);
	if (result.error)
		return result;

	if (!parser_expect(p, token_end)) {
		free(result.ast);
		return parser_error_type(p, token_end);
	}

	return result;
}

static struct parser_result parser_program(struct parser *p)
{
	struct ast *a = ast_create(ast_program);

	while (p->lookahead.type != token_end) {
		struct parser_result stmt = parser_statement(p);
		if (stmt.error) {
			ast_destroy(a);
			return stmt;
		}

		ast_add_child(a, stmt.ast);
	}

	return parser_result_create(a);
}

static struct parser_result parser_statement(struct parser *p)
{
	struct parser_result result;

	if (p->lookahead.type == token_laat)
		result = parser_assignment(p);
	else if (p->lookahead.type == token_print)
		result = parser_print(p);
	else
		return parser_error(p, "assignment or print");

	if (result.error)
		return result;

	if (!parser_expect(p, token_semicolon)) {
		ast_destroy(result.ast);
		return parser_error_type(p, token_semicolon);
	}

	return result;
}

static struct parser_result parser_assignment(struct parser *p)
{
	if (!parser_expect(p, token_laat))
		return parser_error_type(p, token_laat);

	if (p->lookahead.error || p->lookahead.type != token_name)
		return parser_error_type(p, token_name);
	char *assignee = strdup(p->lookahead.name_value);
	parser_consume(p);

	struct parser_result expr = parser_expression(p);
	if (expr.error) {
		free(assignee);
		return expr;
	}

	if (!parser_expect(p, token_zijn)) {
		free(assignee);
		ast_destroy(expr.ast);
		return parser_error_type(p, token_zijn);
	}

	struct ast *ast = ast_create(ast_assign);
	ast_add_child(ast, ast_create_name(assignee));
	ast_add_child(ast, expr.ast);

	return parser_result_create(ast);

}

static struct parser_result parser_print(struct parser *p)
{
	if (!parser_expect(p, token_print))
		return parser_error_type(p, token_print);

	struct parser_result expr = parser_expression(p);
	if (expr.error)
		return expr;

	if (!parser_expect(p, token_uit))
		return parser_error_type(p, token_uit);

	struct ast *ast = ast_create(ast_print);
	ast_add_child(ast, expr.ast);

	return parser_result_create(ast);
}

static struct parser_result parser_expression(struct parser *p)
{
	struct parser_result result = parser_mul(p);
	if (result.error)
		return result;

	for (;;) {
		if (p->lookahead.error) {
			ast_destroy(result.ast);
			return parser_error_lookahead(p);
		}

		struct ast *new_parent;
		if (p->lookahead.type == token_plus)
			new_parent = ast_create(ast_plus);
		else if (p->lookahead.type == token_minus)
			new_parent = ast_create(ast_minus);
		else
			break;

		parser_consume(p);

		struct parser_result mul = parser_mul(p);
		if (mul.error) {
			ast_destroy(result.ast);
			return mul;
		}

		ast_add_child(new_parent, result.ast);
		ast_add_child(new_parent, mul.ast);

		result.ast = new_parent;
	}

	return result;
}

static struct parser_result parser_mul(struct parser *p)
{
	struct parser_result result = parser_value(p);
	if (result.error)
		return result;

	for (;;) {
		if (p->lookahead.error) {
			ast_destroy(result.ast);
			return parser_error_lookahead(p);
		}

		struct ast *new_parent;
		if (p->lookahead.type == token_star)
			new_parent = ast_create(ast_star);
		else if (p->lookahead.type == token_slash)
			new_parent = ast_create(ast_slash);
		else
			break;

		parser_consume(p);

		struct parser_result value = parser_value(p);
		if (value.error) {
			ast_destroy(result.ast);
			return value;
		}

		ast_add_child(new_parent, result.ast);
		ast_add_child(new_parent, value.ast);

		result.ast = new_parent;
	}

	return result;
}

static struct parser_result parser_value(struct parser *p)
{
	if (p->lookahead.error)
		return parser_error_lookahead(p);

	struct ast *result;

	switch (p->lookahead.type) {
	case token_number:
		result = ast_create_number(p->lookahead.number_value);
		break;
	case token_name:
		result = ast_create_name(strdup(p->lookahead.name_value));
		break;
	case token_het:
		result = ast_create(ast_het);
		break;
	case token_lparen:
		parser_consume(p);

		struct parser_result expr = parser_expression(p);
		if (expr.error)
			return expr;

		if (!parser_expect(p, token_rparen))
			return parser_error_type(p, token_rparen);

		return parser_result_create(expr.ast);
	default:
		return parser_error(p, "number, name, het, or(");
	}

	parser_consume(p);
	return parser_result_create(result);
}
