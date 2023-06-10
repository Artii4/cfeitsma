#define _GNU_SOURCE

#include "token.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct token token_create(enum token_type type)
{
	return (struct token) {
		.type = type
	};
}

struct token token_create_name(char *name)
{
	return (struct token) {
		.type = token_name,
		.name_value = name
	};
}

struct token token_create_number(double number)
{
	return (struct token) {
		.type = token_number,
		.number_value = number
	};
}

struct token token_create_error(char *error)
{
	return (struct token) {
		.error = error,
		.type = token_none
	};
}

void token_destroy(struct token *token)
{
	if (token->type == token_name)
		free(token->name_value);
	free(token->error);
}

char *token_to_string(struct token token)
{
	char *s;

	if (token.type == token_name)
		asprintf(&s, "<\"%s\">", token.name_value);
	else if (token.type == token_number)
		asprintf(&s, "<%.5f>", token.number_value);
	else
		asprintf(&s, "<%s>", token_type_to_string(token.type));

	return s;
}

char const *token_type_to_string(enum token_type type)
{
	switch (type) {
	case token_plus:
		return "+";
	case token_minus:
		return "-";
	case token_star:
		return "*";
	case token_slash:
		return "/";
	case token_semicolon:
		return ";";
	case token_lparen:
		return "(";
	case token_rparen:
		return ")";
	case token_number:
		return "number";
	case token_name:
		return "name";
	case token_laat:
		return "laat";
	case token_het:
		return "het";
	case token_zijn:
		return "zijn";
	case token_print:
		return "print";
	case token_uit:
		return "uit";
	case token_en:
		return "en";
	case token_end:
		return "end";
	case token_none:
		return "none";
	default:
		assert(false);
	}
}
