#pragma once

enum token_type {
	token_plus,
	token_minus,
	token_star,
	token_slash,
	token_semicolon,
	token_lparen,
	token_rparen,

	token_number,
	token_name,

	token_laat,
	token_het,
	token_zijn,
	token_print,
	token_uit,
	token_en,

	token_end,

	token_none
};

struct token {
	enum token_type type;
	union {
		double number_value;
		char *name_value;
	};
	char *error;
};

struct token token_create(enum token_type type);
struct token token_create_name(char *name);
struct token token_create_number(double number);
struct token token_create_error(char *error);
void token_destroy(struct token *token);
char *token_to_string(struct token token);
const char *token_type_to_string(enum token_type token);
