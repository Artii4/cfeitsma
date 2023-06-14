#include "parser.h"
#include "interpreter.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	char line[1024];
	struct interpreter i = interpreter_create(stdout);

	for (;;) {
		fputs("> ", stdout);
		fflush(stdout);
		fgets(line, sizeof(line), stdin);

		if (strcmp(line, "q\n") == 0)
			break;

		struct parser parser = parser_create(line);
		struct parser_result res = parser_parse(&parser);

		if (res.error) {
			puts(res.error);
			free(res.error);
			parser_destroy(&parser);
			continue;
		}

		interpreter_interpret(&i, res.ast);
		if (i.error)
			puts(i.error);

		ast_destroy(res.ast);
		parser_destroy(&parser);
	}

	interpreter_destroy(&i);

	return 0;
}
