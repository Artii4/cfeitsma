#pragma once

#include <stddef.h>
#include <stdlib.h>

#define array_declare(t, suf)							\
	struct array_##suf {							\
		size_t nelts;							\
		size_t nalloc;							\
		t *elts;							\
	};

#define array_create_declare(t, suf)						\
	static struct array_##suf array_create_##suf()					\
	{									\
		return (struct array_##suf) {					\
			.nelts = 0,						\
			.nalloc = 8,						\
			.elts = malloc(sizeof(t) * 8)				\
		};								\
	}

#define array_destroy_declare(t, suf)						\
	static void array_destroy_##suf(struct array_##suf self)			\
	{									\
		free(self.elts);						\
	}

#define array_push_declare(t, suf)							\
	static t *array_push_##suf(struct array_##suf *self)				\
	{									\
		if (self->nelts >= self->nalloc) {				\
			size_t const nmem = sizeof(t) * self->nelts * 2;	\
			t *new = realloc(self->elts, nmem);			\
			if (new == NULL)					\
				return NULL;					\
										\
			self->elts = new;					\
			self->nalloc = self->nelts * 1.5;			\
		}								\
										\
		return &self->elts[self->nelts++];				\
	}

#define array_all_declare(t, suf)						\
	array_declare(t, suf) 							\
	array_create_declare(t, suf)						\
	array_push_declare(t, suf)						\
	array_destroy_declare(t, suf)
