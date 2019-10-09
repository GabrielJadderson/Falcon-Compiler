#include "peephole.h"
#include <string.h>
#include <stdio.h>
#include "falcon_string.h"
#include "array.h"
#include "logger.h"
#include "memory.h"
#include <stdlib.h>
#include "file_io.h"

//GHJ: this would be easier to do with some intermediate language, we should implement IR.
char* peephole(linked_list* code_list)
{
	linked_list* split_code_list = peephole_split_list_by_newlines(code_list);

	array* code_array = linked_list_to_array(split_code_list);

	array* optimized_array = peephole_optimize_push_pop(code_array);

	linked_list* final_list = array_to_linked_list(optimized_array);

	linked_list_print(&final_list);
	//output to a file.
	file_write_linked_list("output.s", &final_list);

	return "";
}

static linked_list* peephole_split_list_by_newlines(linked_list* code_list)
{
	char* code_list_str = linked_list_serialize_to_char_array(&code_list);
	char* temp = NULL;

	linked_list* result = linked_list_new();

	string_builder* sb = string_builder_new();

	//GHJ: strtok will store internally the first argument code_list_str,
	//subsequent calls with "NULL" as argument will increment to the next newline.
	//to reset, you can pass in a non-null string.
	temp = strtok(code_list_str, "\r\n");
	while (temp != NULL)
	{
		string_builder_append_chars(sb, temp);
		string_builder_append_chars(sb, "\n"); //this is needed because strtok removes the actual newline it splits, but we're still interested in it.
		linked_list_add_last(&result, string_builder_copy_to_char_array(sb));
		string_builder_clear(sb);
		temp = strtok(NULL, "\r\n"); //set/advance temp to the next newline terminated string.
	}
	return result;
}


/*
 * takes the input such as:
 * pushq $5
 * pushq $4
 * popq %rbx
 * popq %rax
 * and replaces with
 * movq $4, %rbx
 * movq $5, %rax
 * Do note that the input code_list must be split by newlines.
*/
static array* peephole_optimize_push_pop(array* code_array)
{
	size_t break_point = 0;
	size_t code_list_length = code_array->length;
	size_t start_index = 0;
	char did_fail = 0;

	array* window = NULL;

	//1. find the instruction: pushq $
	//2. request a 4 length window
	//3. process the window.
	for (size_t i = 0; i < code_list_length; ++i)
	{
		char* instruction = array_at(code_array, i);
		if (substring_cmp(instruction, "pushq", 0, 5) == 0)
		{
			if (substring_cmp(instruction, "$", 6, 1) == 0)
			{
				if (break_point == 0)
				{
					window = peephole_retrieve_window(code_array, i, 4);
					start_index = i;
					break_point = i;
				}
				else
				{
					if (i > (break_point + 1))  //fail not allowed.
					{
						did_fail = 1;
						break;
					}
				}
			}
		}
	}

	if (!did_fail)
	{
		size_t window_length = window->length;
		if (window_length > 4)
		{
			log_internal_error(__FILE__, __func__, __LINE__, "unsupported peephole optimization.");
		}
		//get the instructions.
		char* push_instruction_1 = array_at(window, 0);
		char* push_instruction_2 = array_at(window, 1);

		char* pop_instruction_1 = array_at(window, 2);
		char* pop_instruction_2 = array_at(window, 3);


		//split the push instructions and get their value.
		char* push_instruction_1_temp = substring_ins(push_instruction_1, 7);
		char* push_instruction_1_value = strtok(push_instruction_1_temp, "\r\n");


		char* push_instruction_2_temp = substring_ins(push_instruction_2, 7);
		char* push_instruction_2_value = strtok(push_instruction_2_temp, "\r\n");



		//split the pop instructions and get their value.
		char* pop_instruction_1_temp = substring_ins(pop_instruction_1, 6);
		char* pop_instruction_1_value = strtok(pop_instruction_1_temp, "\r\n");


		char* pop_instruction_2_temp = substring_ins(pop_instruction_2, 6);
		char* pop_instruction_2_value = strtok(pop_instruction_2_temp, "\r\n");


		string_builder* sb = string_builder_new();

		string_builder_append_chars(sb, "movq $");
		string_builder_append_chars(sb, push_instruction_2_value);
		string_builder_append_chars(sb, ", %");
		string_builder_append_chars(sb, pop_instruction_1_value);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "movq $");
		string_builder_append_chars(sb, push_instruction_1_value);
		string_builder_append_chars(sb, ", %");
		string_builder_append_chars(sb, pop_instruction_2_value);
		string_builder_append_chars(sb, "\n");
		char* result = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);




		//initiate replacement

		array* array_result = array_new();
		//remove old values by creating a new array and inserting a splitting of the old.
		for (size_t i = 0; i < code_list_length; ++i)
		{
			if (i == start_index)
			{
				array_push(array_result, result);
				i = i + 3; //skip the next 3 instructions.
			}
			else
			{
				void* element = array_at(code_array, i);
				array_push(array_result, element);
			}
		}

		//cleanup
		free(push_instruction_1_temp);
		free(push_instruction_2_temp);
		free(pop_instruction_1_temp);
		free(pop_instruction_2_temp);
		array_delete(window);

		return array_result;
	}
	return NULL;
}


array* peephole_retrieve_window(array* code_array, size_t index, size_t window_length)
{
	if (!code_array) return NULL;
	if (index >= code_array->length || index >= (code_array->length - window_length)) return NULL;
	array * result = array_new();
	for (size_t i = 0; i < window_length; ++i)
	{
		array_push(result, array_at(code_array, index + i));
	}
	return result;
}

int substring_cmp(char* str_1, char* str_2, size_t offset, size_t length)
{
	char* str_3 = Calloc(length + 1, 1);
	strncpy(str_3, str_1 + offset, length);
	int result = strcmp(str_3, str_2);
	free(str_3);
	return result;
}

char* substring_ins(char* array, size_t offset)
{
	size_t len = strlen(array);
	char* str_3 = Calloc(len - offset + 1, 1);
	strncpy(str_3, array + offset, len - offset);
	return str_3;
}