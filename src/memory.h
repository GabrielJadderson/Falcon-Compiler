#include <stddef.h>

void *Malloc(size_t n);
#define NEW(type) (type *)Malloc(sizeof(type))

void* Calloc(size_t amount, size_t size);
void* Realloc(void* ptr_address, size_t size);
