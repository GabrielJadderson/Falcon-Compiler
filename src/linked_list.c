/* linked_list.c
* Authors: Gabriel H. Jadderson
* Created:  14/04/2018
*/

#include <stddef.h>
#include "linked_list.h"
#include "memory.h"
#include <stdlib.h>
#include "logger.h"
#include <stdio.h>
#include "falcon_string.h"
#include <string.h>

linked_list* linked_list_new()
{
	linked_list* ptr = Malloc(sizeof(linked_list));
	ptr->root = Malloc(sizeof(void*)); //allocate space for pointer to pointer root and tail
	ptr->tail = Malloc(sizeof(void*));
	ptr->Length = 0;
	return ptr;
}

static inline linked_list_node* linked_list_node_init()
{
	linked_list_node* ptr = Malloc(sizeof(linked_list_node));
	ptr->data = 0;
	ptr->next = NULL;
	ptr->previous = NULL;
	return ptr;
}

size_t linked_list_length(linked_list** list)
{
	if (list && *list)
	{
		return (*list)->Length;
	}
	else return -1;
}

/* O(1). returns a pointer to the void* element. returns NULL if nothing found. */
void* linked_list_remove_first(linked_list** list)
{
	if (*list)
	{
		linked_list_node** referencePointer = (*list)->root; //store what's pointed by root
		void* element = (*referencePointer)->data; //get the data of roots pointer.
		if ((*list)->Length == 1)//if tail equals root, then there's only 1 element in the list.
		{
			(*list)->Length--;
			free(*referencePointer); //free the pointer that root is pointing at.
			*(*list)->root = NULL; //reset both root and tail to null, since mem is now free'd
			*(*list)->tail = NULL;
		}
		else
		{
			(*list)->Length--;
			linked_list_node* next = (*referencePointer)->next;
			next->previous = NULL;
			free(*(*list)->root);
			*referencePointer = next; //update root to the next one, such that we are no longer pointing to free'd mem.
		}
		return element;
	}
	else return NULL;
}

/* O(1). returns a pointer to the void* element. returns NULL if nothing found.*/
void* linked_list_remove_last(linked_list** list)
{
	if (*list)
	{
		linked_list_node** referencePointer = (*list)->tail; //store what's pointed by the tail
		void* element = (*referencePointer)->data; //get the data of tails pointer.
		if ((*list)->Length == 1) //if last
		{
			(*list)->Length--;
			free(*referencePointer); //free the pointer that root is pointing at.
			*(*list)->root = NULL; //reset both root and tail to null, since mem is now free'd
			*(*list)->tail = NULL;
		}
		else
		{
			linked_list_node* prev = (*referencePointer)->previous;
			prev->next = NULL; //nulify next
			free(*(*list)->tail); //free whats pointed to by root
			*referencePointer = prev; //update the tail to the previous one, such that we are no longer pointing to free'd mem.
			(*list)->Length--;
		}
		return element;
	}
	else return NULL;
}


// O(1)
void linked_list_add_first(linked_list** list, void* element)
{
	if (*list)
	{
		if ((*list)->Length == 0) //if entirely new or empty.
		{
			linked_list_node* newList = linked_list_node_init();
			newList->data = element;
			newList->next = NULL;
			newList->previous = NULL;
			*(*list)->root = newList;
			(*list)->Length = 1;
			*(*list)->tail = newList;
			return;
		}
		else //else if the list contains atleast one element
		{
			linked_list_node* newList = linked_list_node_init();

			newList->data = element;
			newList->next = *(*list)->root;
			(*(*list)->root)->previous = newList;
			*(*list)->root = newList; //finally update root to the new list.
			(*list)->Length++;
		}
	}
}


// O(1)
void linked_list_add_last(linked_list** list, void* element)
{
	if (*list)
	{
		if ((*list)->Length == 0) //if entirely new or empty.
		{
			linked_list_node* newList = linked_list_node_init();
			newList->data = element;
			newList->next = NULL;
			newList->previous = NULL;
			newList->func = NULL;
			*(*list)->root = newList;
			(*list)->Length = 1;
			*(*list)->tail = newList;
			return;
		}
		else //else if the list contains atleast one element
		{
			linked_list_node* newList = linked_list_node_init();
			newList->data = element;
			newList->func = NULL;
			newList->previous = *(*list)->tail;
			(*(*list)->tail)->next = newList;
			*(*list)->tail = newList; //finally update tail to the new list.
			(*list)->Length++;
		}
	}
}

// O(n)
void* linked_list_remove_element(linked_list** list, void* element)
{
	linked_list_node** referencePointer = (*list)->root;

	int found = -1;
	while ((*referencePointer))
	{
		if ((*referencePointer)->data == element)
		{
			found = 0;
			break;
		}
		else
			referencePointer = &(*referencePointer)->next;
	}

	if (found == 0)
	{
		linked_list_node* match = *referencePointer;
		//match->next->previous = match->previous->next; //update the pointers accordingly
		//match->previous->next = match->next;

		if ((*referencePointer) == *(*list)->tail) //if reached tail.
			* (*list)->tail = (*referencePointer)->next;

		*referencePointer = (*referencePointer)->next;
		if (*referencePointer != NULL)
			(*referencePointer)->previous = (*referencePointer)->previous->previous;

		void* result = match->data;

		free(match);
		return result;
	}
	return NULL;
}

/* Clears and deletes everything.*/
void linked_list_full_cleanup(linked_list * *list)
{
	if (*list)
	{
		if (((*list)->Length == 0 && *(*list)->root == NULL && *(*list)->tail == NULL)) //if newly created or has 0 elements just cleanup easily.
		{
			free((*list)->root); //free root and tail then the list.
			free((*list)->tail);
			free(*list);
			return;
		}
		else if ((*list)->Length > 0) //otherwise loop and clean up everything
		{
			linked_list_node** referencePointer = (*list)->root;
			while (*referencePointer)
			{
				linked_list_node* ptr = *referencePointer;
				referencePointer = &(*referencePointer)->next; //continue down the list until null
				//free what's stored in them
				if (ptr->data)
					free(ptr->data);
				ptr->data = NULL;
				free(ptr);
			}
			free((*list)->root); //free root and tail then the list.
			free((*list)->tail);
			free(*list);
		}
		else
		{
			log_internal_error(__FILE__, __func__, __LINE__, "Failed to delete and clean the linked list because it has been abused.");
		}
	}
}

/* Cleans up the internal structure of the linked_list leaving all allocated data/elements untouched.*/
void linked_list_cleanup(linked_list * *list)
{
	if (*list)
	{
		if (((*list)->Length == 0 && *(*list)->root == NULL && *(*list)->tail == NULL)) //if newly created or has 0 elements just cleanup easily.
		{
			free((*list)->root); //free root and tail then the list.
			free((*list)->tail);
			free(*list);
			return;
		}
		else if ((*list)->Length > 0) //otherwise loop and clean up everything
		{
			linked_list_node** referencePointer = (*list)->root;
			while (*referencePointer)
			{
				linked_list_node* ptr = *referencePointer;
				referencePointer = &(*referencePointer)->next; //continue down the list until null
				free(ptr);
			}
			free((*list)->root); //free root and tail then the list.
			free((*list)->tail);
			free(*list);
		}
		else
		{
			log_internal_error(__FILE__, __func__, __LINE__, "Failed to delete and clean the linked list because it has been abused.");
		}
	}
}

/* takes in a function pointer and invokes that function on every element in the linked-list
 * the function must have a single parameter of type void* to take in the element. */
void linked_listed_functional_iterator(linked_list * *list, void* func(void*))
{
	if (!list) return;

	linked_list_node** referencePointer = (*list)->root;
	while (*referencePointer)
	{
		linked_list_node* ptr = *referencePointer;
		referencePointer = &(*referencePointer)->next; //continue down the list until null
		func(ptr->data);
	}
}


//assumes the list is populated with char*
void linked_list_print(linked_list **list)
{
	if (!list) return;

	linked_list_node** referencePointer = ((*list)->root);
	while (*referencePointer)
	{
		linked_list_node* ptr = *referencePointer;
		referencePointer = &(*referencePointer)->next; //continue down the list until null
		printf("%s", (char*)ptr->data);
	}
}

void linked_list_concatenation(linked_list** list_1, linked_list** list_2)
{
	if (*list_2)
	{
		if ((*list_2)->Length == 0) //if list_2 is entirely new or empty.
		{
			return;
		}
		else if ((*list_1)->Length == 0)
		{
			*(*list_1)->root = *(*list_2)->root;
			*(*list_1)->tail = *(*list_2)->tail;
			(*list_1)->Length = (*list_2)->Length;
		}
		else //else if the list_2 contains atleast one element
		{
			(*(*list_2)->root)->previous = *(*list_1)->tail;
			(*(*list_1)->tail)->next = *(*list_2)->root;
			*(*list_1)->tail = *(*list_2)->tail; //finally update tail to the new list.
			(*list_1)->Length += (*list_2)->Length;
		}
	}
}


/* Get and set offset is used to insert offset of parameter or variable
 * into symbol table for later reference */
void linked_list_set_offset(linked_list** offset_stack, int num)
{
	(*(*offset_stack)->tail)->offset -= num;
}

int linked_list_get_offset(linked_list** offset_stack)
{
	return (*(*offset_stack)->tail)->offset;
}


void linked_list_insert_function(linked_list** list, void* element){
	//we assume that linke_list_add_last has been called before this function
	//so list is never empty, thus we just look at list->func and check if it exists, and insert to it.
	if (*list)
	{
		if ( !((*(*list)->tail)->func) ) //if empty func linked_list in tail.
		{
			linked_list* sub_list = linked_list_new();
			linked_list_add_last(&sub_list, element);

			(*(*list)->tail)->func = sub_list;
			return;
		}
		else //else if the list contains an linked_list in tail, add to it
		{
			linked_list_add_last(&((*(*list)->tail)->func), element);
		}
	}
}

//we assume that a linked list function is only removed after it has been created
linked_list** linked_list_remove_function(linked_list** list){
	linked_list** result = &((*(*list)->tail)->func);
	linked_list_remove_last(list);
	return result;
}

//assuming the list contains char*'s
char* linked_list_serialize_to_char_array(linked_list** list)
{
	if (!list) return NULL;

	string_builder* sb = string_builder_new();

	linked_list_node** referencePointer = ((*list)->root);
	while (*referencePointer)
	{
		linked_list_node* ptr = *referencePointer;
		referencePointer = &(*referencePointer)->next; //continue down the list until null
		string_builder_append_chars(sb, (char*)ptr->data);
	}

	char* str = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return str;
}

linked_list* linked_list_split_list_by_newlines(linked_list* code_list)
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