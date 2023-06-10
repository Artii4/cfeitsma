#include "array.h"
#include <stdlib.h>

struct array *array_create(size_t size)
{
	struct array *a = malloc(sizeof(struct array));
	if (a == NULL)
		return NULL;

	int const nalloc = 8;

	*a = (struct array) {
		.nelts = 0,
		.nalloc = nalloc,
		.size = size,
		.elts = malloc(size * nalloc)
	};

	if (a->elts == NULL) {
		free(a);
		return NULL;
	}

	return a;
}

void array_destroy(struct array *a)
{
	free(a->elts);
	free(a);
}

void *array_push(struct array *a)
{
	if (a->nelts >= a->nalloc) {
		void *new = realloc(a->elts, a->size * a->nelts * 2);
		if (new == NULL)
			return NULL;

		a->elts = new;
		a->nalloc = a->nelts * 2;
	}

	return (unsigned char *)a->elts + (a->nelts++ * a->size);
}
