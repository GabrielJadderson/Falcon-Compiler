/*
* stack.c
* Authors: Gabriel H. Jadderson
* Created: 24/02/2019
*/

#include "stack.h"
#include "logger.h"
#include "memory.h"
#include <stdlib.h>


stack* stack_new(size_t capacity)
{
	stack* stack_ptr = (stack*)Malloc(sizeof(stack));
	stack_ptr->capacity = capacity;
	stack_ptr->top = 0;
	stack_ptr->contents = (void*)Malloc(sizeof(void*) * capacity);
	return stack_ptr;
}

size_t stack_capacity(stack* stack)
{
	if (stack)
	{
		return stack->capacity;
	}
	return 0;
}

size_t stack_size(stack* stack)
{
	if (stack)
	{
		return stack->top;
	}
	return 0;
}

//the return is a boolean
char stack_is_full(stack * stack)
{
	if (stack)
	{
		return stack->top == stack->capacity;
	}
	return 0;
}

void stack_push(stack * stack, void* element)
{
	if (stack)
	{
		if (!stack_is_full(stack))
			stack->contents[stack->top++] = element;
	}
	else log_internal_error(__FILE__, __func__, __LINE__, "Stack was null.");
}

void* stack_pop(stack * stack)
{
	if (stack)
	{
		if (!stack_is_full(stack))
			return stack->contents[--stack->top]; //pre-decrement first.
	}
	else log_internal_error(__FILE__, __func__, __LINE__, "Stack was null.");
	return NULL;
}

/**
 * pops all elements free'ing them from the stack.
 * and deletes the stack itself.
 * The Stack Cannot be reused after this.
 */
void stack_destroy(stack * stack)
{
	if (stack)
	{
		do
		{
			void* ele = stack_pop(stack);
			if (ele)
			{
				free(ele);
				ele = NULL;
			}
		} while (stack->top > 0);
		free(stack->contents);
		free(stack);
	}
}