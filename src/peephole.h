#pragma once
#include "linked_list.h"
#include "array.h"

//GHJ: this would be easier to do with some intermediate language, we should implement IR.
char* peephole(linked_list* code_list);

static linked_list* peephole_split_list_by_newlines(linked_list* code_list);
static array* peephole_optimize_push_pop(array* code_array);

static int substring_cmp(char* str_1, char* str_2, size_t offset, size_t length);
static char* substring_ins(char* array, size_t offset);
static array* peephole_retrieve_window(array* code_array, size_t index, size_t window_length);

typedef struct peephole_replacement_node
{
	size_t index; //the index of the instruction to be replaced.
	char* replacement;
	enum
	{
		PEEP_PUSH_POP,
		PEEP_CONST,
		NONE,
	} kind;
} peephole_replacement_node;
