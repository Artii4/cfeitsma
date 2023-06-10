#include "parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool parser_errs(char *input)
{
	struct parser parser = parser_create(input);
	struct parser_result res = parser_parse(&parser);

	if (!res.error) {
		parser_destroy(&parser);
		ast_destroy(res.ast);
		return false;
	}

	parser_destroy(&parser);
	return true;
}

bool parser_gives(char *input, char *ast)
{
	struct parser parser = parser_create(input);
	struct parser_result res = parser_parse(&parser);

	if (res.error) {
		parser_destroy(&parser);
		return false;
	}

	char *s = ast_to_string(res.ast);

	if (strcmp(s, ast)) {
		free(s);
		ast_destroy(res.ast);
		parser_destroy(&parser);
		return false;
	}

	free(s);
	parser_destroy(&parser);
	ast_destroy(res.ast);
	return true;
}

int main(void)
{
	assert(parser_errs("laat x 2 + laat zijn;"));
	assert(parser_errs("laat @ x zijn;"));
	assert(parser_errs("laat pi @ zijn;"));
	assert(parser_errs("print 2 + 2 uit"));
	assert(parser_errs("laat tau pi * pi * zijn;"));

	assert(parser_gives("", "program"));
	assert(parser_gives("print pi uit;", "program (print (\"pi\"))"));
	assert(parser_gives("print pi uit; print tau uit;",
			    "program (print (\"pi\"), print (\"tau\"))"));
	assert(parser_gives("print 2 + 2 uit;",
			    "program (print (+ (2.00000, 2.00000)))"));
	assert(parser_gives("laat tau pi * pi zijn;",
			    "program (= (\"tau\", * (\"pi\", \"pi\")))"));

	puts("Tests passed.");
}
