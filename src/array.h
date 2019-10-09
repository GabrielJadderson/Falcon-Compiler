#pragma once
#include <stddef.h>
#include "linked_list.h"

typedef struct array
{
	size_t length;
	size_t capacity;
	void** buffer;
} array;


array* array_new();
array* array_new_as(size_t capacity);
void array_delete(array* array);
void array_delete_keep_elements(array* array);

size_t array_push(array* array, void* element);
void* array_pop(array* array);

size_t array_get(array* array, void* element);
void* array_at(array* arr, size_t index);
void* array_replace_at(array* arr, size_t index, void* value);
void array_add_at(array* arr, size_t index, void* value);
void* array_remove_at(array* arr, size_t index);

void array_merge(array* A, array* B);
void array_merge_reverse(array* A, array* B);

void array_resize(array* arr, size_t size);

array* linked_list_to_array(linked_list* list);
linked_list* array_to_linked_list(array* code_array);
