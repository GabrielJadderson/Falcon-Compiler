#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "logger.h"

void *Malloc(size_t n)
{
	void *p;
	if (!(p = malloc(n)))
	{
		fprintf(stderr, "Malloc(%lu) failed.\n", n);
		fflush(stderr);
		abort();
	}
	return p;
}

void* Calloc(size_t amount, size_t size)
{
	void* p = calloc(amount, size);
	if (!p)
	{
		fprintf(stderr, "Calloc(%lu,%lu) failed.\n", amount, size);
		fflush(stderr);
		abort();
	}
	return p;
}

void* Realloc(void* ptr_address, size_t size)
{
	void* p = realloc(ptr_address, size);
	if (!p)
	{
		fprintf(stderr, "Realloc(%p,%lu) failed.\n", ptr_address, size);
		fflush(stderr);
		abort();
	}
	return p;
}
