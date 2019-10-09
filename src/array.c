/* array.c
* Authors: Gabriel H. Jadderson
* Created:  24/02/2019
*/

#include "array.h"
#include <stddef.h>
#include "logger.h"
#include "memory.h"
#include <stdlib.h>
#include <string.h>

array* array_new_as(size_t capacity)
{
	array* arr = (array*)Malloc(sizeof(array));
	arr->length = 0;
	arr->capacity = capacity;
	arr->buffer = (void*)Malloc(sizeof(void*) * capacity);
	return arr;
}

array* array_new() { return array_new_as(16); }

/* PUSHES AN ELEMENT AT THE END */
size_t array_push(array* array, void* element)
{
	if (array->length == array->capacity)
		array_resize(array, array->capacity << 1); //amortized run time power of two.

	array->buffer[array->length] = element;
	return array->length++;
}

/* POPS AN ELEMENT FROM THE END */
void* array_pop(array * array)
{
	if ((array->length) != 0)
	{
		return array->buffer[--array->length];
	}
	else { log_internal_error(__FILE__, __func__, __LINE__, "POPPING FROM AN EMPTY ARRAY!"); }
	return NULL;
}


size_t array_get(array * array, void* element)
{
	for (size_t i = 0; i < array->length; i++)
	{
		if (array->buffer[i] == element)
			return i;
	}
	return -1;
}

void* array_at(array * arr, size_t index)
{
	if (index > arr->capacity) return NULL;
	return arr->buffer[index];
}

/* removes an element at the specified index in the array and decrements the array size.
 * returns the old at that index value. returns null if the old value was null. O(1) */
void* array_remove_at(array * arr, size_t index)
{
	if (index > arr->capacity) return NULL;
	void* el = arr->buffer[index];
	arr->buffer[index] = NULL;
	arr->length--;
	return el;
}

/* adds an element at the specified index in the array and increments the array size. O(1) */
void array_add_at(array * arr, size_t index, void* value)
{
	if (index > arr->capacity) return;
	arr->buffer[index] = value;
	arr->length++;
}

/* replaces an index in the array with a new one and returns the old value,
 * returns null pointer if the old value was null. O(1) */
void* array_replace_at(array * arr, size_t index, void* value)
{
	if (index > arr->capacity) return NULL;
	void* old_data = arr->buffer[index];
	arr->buffer[index] = value;
	return old_data;
}


/* Deletes everything, the array structures AND the inserted elements. O(n) */
void array_delete(array * array)
{
	if (array)
	{
		size_t length = array->length;
		for (size_t i = 0; i < length; i++)
		{
			void* ele = array_pop(array);
			if (ele)
			{
				free(ele);
				ele = NULL;
			}
		}
		free(array->buffer);
		free(array);
		array = NULL;
	}
}

/* deletes only the array structure and does not deallocate the inserted items.  O(1) */
void array_delete_keep_elements(array * array)
{
	if (array)
	{
		free(array->buffer);
		free(array);
		array = NULL;
	}
}

/* Merges array B into A. DESTROYING B in the process.
 * in other words, takes all the elements in B and puts them in A then free's B.
 * <p>Example: A = {5, 3, 1, 4} + B = {0, 1} -> A then becomes {5, 3, 1, 4,  0, 1}
 * O(n) where n is B.length */
void array_merge(array * A, array * B)
{
	if (A && B)
	{
		size_t b_len = B->length;
		for (size_t i = 0; i < b_len; i++)
		{
			array_push(A, B->buffer[i]);
		}
		free(B->buffer);
		free(B);
	}
}

/* Merges array B into A in reverse. DESTROYING B in the process.
 * in other words, takes all the elements in B and puts them in A then free's B.
 * Doing this in reverse implies that the elements from B are taking from the back and put into A.
 * Example: A = {5, 3, 1, 4} + B = {0, 1} -> A then becomes {5, 3, 1, 4, 1, 0}
 * O(n) where n is B.length */
void array_merge_reverse(array * A, array * B)
{
	if (A && B)
	{
		size_t b_len = B->length;
		for (size_t i = 0; i < b_len; i++)
		{
			array_push(A, array_pop(B));
		}
		free(B->buffer);
		free(B);
	}
}


void array_resize(array * arr, size_t size)
{
	if (arr->capacity < size) //increase
	{
		arr->capacity = size;
		arr->buffer = Realloc(arr->buffer, sizeof(void*) * size);
	}
	else //shrink
	{
		if (arr->length > size) //shrink
		{ //delete the difference and resize
			for (size_t i = size; i < arr->length; ++i)
			{
				void* ele = array_at(arr, i);
				if (ele)
				{
					free(ele);
					ele = NULL;
				}
			}
			Realloc(arr->buffer, sizeof(void*) * size);
			arr->length = size;
			arr->capacity = size;
		}
		else if (arr->length < size) //increase
		{ //then just expand the array
			arr->capacity = size;
			arr->buffer = Realloc(arr->buffer, sizeof(void*) * size);
		}
	}
}

/* returns the index of an element in the array. O(n) */
size_t array_find(array * arr, void* element)
{
	if (arr == NULL || element == NULL) return 0;

	for (size_t i = 0; i < arr->length; i++)
	{
		if (arr->buffer[i] == element)
		{
			return i;
		}
	}
	return 0;
}


array* linked_list_to_array(linked_list* list)
{
	if (!list) return NULL;
	array* array = array_new_as(list->Length);
	size_t list_length = list->Length;
	for (size_t i = 0; i < list_length; ++i)
	{
		void* element = linked_list_remove_first(&list);
		array_push(array, element);
	}
	//populate the list again.
	for (size_t i = 0; i < list_length; ++i)
	{
		void* element = array_at(array, i); //peek
		linked_list_add_last(&list, element);
	}
	return array;
}

linked_list* array_to_linked_list(array* code_array)
{
	if (!code_array) return NULL;
	linked_list* result = linked_list_new();
	size_t array_len = code_array->length;
	for (size_t i = 0; i < array_len; ++i)
	{
		void* element = array_remove_at(code_array, i);
		linked_list_add_last(&result, element);
	}
	return result;
}
