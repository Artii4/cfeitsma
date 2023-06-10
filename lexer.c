#define _GNU_SOURCE

#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void lexer_consume(struct lexer *l);
static char lexer_peek(struct lexer *l);
static struct token lexer_name(struct lexer *l);
static struct token lexer_number(struct lexer *l);

struct lexer lexer_create(char *input)
{
	return (struct lexer) {
		.input = input,
		.index = 0
	};
}

struct token lexer_next_token(struct lexer *l)
{
	while (isspace(lexer_peek(l)))
		lexer_consume(l);

	enum token_type type;

	switch (lexer_peek(l)) {
	case '+':
		type = token_plus;
		break;
	case '-':
		type = token_minus;
		break;
	case '*':
		type = token_star;
		break;
	case '/':
		type = token_slash;
		break;
	case '(':
		type = token_lparen;
		break;
	case ')':
		type = token_rparen;
		break;
	case ';':
		type = token_semicolon;
		break;
	case '\0':
		type = token_end;
		break;
	default:
		if (isdigit(lexer_peek(l))) {
			return lexer_number(l);
		} else if (isalpha(lexer_peek(l))) {
			return lexer_name(l);
		} else {
			char *error;
			asprintf(&error, "Invalid character: '%c'",
				 lexer_peek(l));
			lexer_consume(l);
			return token_create_error(error);
		}
	}

	lexer_consume(l);

	return token_create(type);
}

static void lexer_consume(struct lexer *l)
{
	if (l->input[l->index])
		l->index++;
}

static char lexer_peek(struct lexer *l)
{
	return l->input[l->index];
}

static struct token lexer_name(struct lexer *l)
{
	int const bufsize = 100;
	char buf[bufsize + 1] = { 0 };

	for (int i = 0; isalpha(lexer_peek(l)); i++) {
		if (i >= bufsize) {
			char *err;
			asprintf(&err,
				 "Name starting with \"%.10s\" is too long!",
				 buf);

			while (isalpha(lexer_peek(l)))
				lexer_consume(l);

			return token_create_error(err);
		}

		buf[i] = lexer_peek(l);
		lexer_consume(l);
	}

	if (!strcmp(buf, "laat"))
		return token_create(token_laat);
	else if (!strcmp(buf, "het"))
		return token_create(token_het);
	else if (!strcmp(buf, "zijn"))
		return token_create(token_zijn);
	else if (!strcmp(buf, "print"))
		return token_create(token_print);
	else if (!strcmp(buf, "uit"))
		return token_create(token_uit);
	else if (!strcmp(buf, "en"))
		return token_create(token_en);
	else
		return token_create_name(strdup(buf));
}

static struct token lexer_number(struct lexer *l)
{
	double n = 0;

	while (isdigit(lexer_peek(l))) {
		n *= 10;
		n += lexer_peek(l) - '0';
		lexer_consume(l);
	}

	return token_create_number(n);
}
