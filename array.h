#pragma once

#include <stddef.h>

struct array {
	void *elts;
	size_t nelts;
	size_t nalloc;
	size_t size;
};

struct array *array_create(size_t size);
void array_destroy(struct array *a);
void *array_push(struct array *a);
