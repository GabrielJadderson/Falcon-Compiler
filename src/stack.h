/*
* stack.h
* Authors: Gabriel H. Jadderson
* Created: 24/02/2019
*/

#pragma once
#include <stddef.h>

typedef struct stack
{
	//an index referring to the object on top of the stack.
	size_t top; //This is not the first element in the array, but instead the last element.
	size_t capacity; //max elements
	void** contents;
} stack;


stack*	stack_new(size_t capacity);
void	stack_push(stack* stack, void* element);
void*	stack_pop(stack* stack);
void	stack_destroy(stack* stack);

//util
size_t	stack_capacity(stack* stack);
size_t	stack_size(stack* stack);
char	stack_is_full(stack* stack);