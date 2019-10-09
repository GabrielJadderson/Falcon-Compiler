#pragma once

typedef struct linked_list_node
{
	struct linked_list_node* next;
	struct linked_list_node* previous;
	void* data;

	//used to keep track of functions
	struct linked_list* func;
	//used to keep track of variable and parameter offset
	int offset;
} linked_list_node;

// Data structure for the linked list
typedef struct linked_list
{
	size_t Length; //how many valid elements we have.
	struct linked_list_node** root;
	struct linked_list_node** tail;
} linked_list;

//Initializes an empty linked list, and returns a pointer to the new list.
linked_list* linked_list_new();

/* returns the Length, in O(1) run time
 * returns -1 if the linked list if null or is not initialized. */
size_t linked_list_length(linked_list** list);

void* linked_list_remove_first(linked_list** list);
void* linked_list_remove_last(linked_list** list);

void linked_list_add_first(linked_list** list, void* element);
void linked_list_add_last(linked_list** list, void* element);


/* Remove the first occurrence of the element from the list.
 * If the element is not found, -1 is returned, if found and removed 0 is returned. */
void* linked_list_remove_element(linked_list** list, void* element);


void linked_list_full_cleanup(linked_list** list);
void linked_list_cleanup(linked_list** list);


void linked_listed_functional_iterator(linked_list** list, void* func(void*));

void linked_list_print(linked_list **list);

void linked_list_concatenation(linked_list** list_1, linked_list** list_2);
void linked_list_set_offset(linked_list** offset_stack, int num);

int linked_list_get_offset(linked_list** offset);
void linked_list_insert_function(linked_list** list, void* element);

linked_list** linked_list_remove_function(linked_list** list);
char* linked_list_serialize_to_char_array(linked_list** list);

linked_list* linked_list_split_list_by_newlines(linked_list* code_list);

