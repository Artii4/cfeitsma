#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char line[1024];

	for (;;) {
		fgets(line, sizeof(line), stdin);

		struct parser parser = parser_create(line);
		struct parser_result res = parser_parse(&parser);

		if (res.error) {
			puts(res.error);
			free(res.error);
		} else {
			char *tree = ast_to_string(res.ast);
			puts(tree);
			free(tree);
			ast_destroy(res.ast);
		}

		parser_destroy(&parser);
	}

	return 0;
}
