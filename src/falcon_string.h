#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct string
{
	size_t Length;
	char* Text;
} string, String, falcon_string;

typedef struct string_builder
{
	string* str;
	//array* list;
} string_builder, String_Builder;

char* string_substring_from_left(char* array, size_t offset);
char* string_substring_from_right(char* array, size_t offset);

char *string_substring(char *string, size_t position, size_t length);
string* string_append_at(string* original, string* appendee, size_t index);
char* string_append_chars_at(char* original, char* appendee, size_t index);
char* string_int_to_string(int integer);

//Heap
string* string_heap_new(char* text);
string* string_heap_new_with_int(int integer);
string* string_heap_new_by(char* text, size_t length);
string* string_heap_new_memcpy(char* text);
string* string_heap_new_allocated(char** text, size_t length);
void string_heap_delete(string* str);
void string_heap_delete_struct(string* str);

//util
void string_print(string* str);
string* string_concatenate_from_chars(char* stringA, char* stringB);
string* string_concatenate(string* stringA, string* stringB);
char* string_concatenate_to_chars(string* stringA, string* stringB);
char* string_concat(char* stringA, char* stringB);
//======================= string_builder ==========================
//Initializes a new instance of the string_builder
string_builder* string_builder_new();
//Initializes a new instance of the string_builder using the specified string.
string_builder* string_builder_new_with_string(string* string);

/* Appends a copy of the specified string to the specified string_builder struct.
 * returns the same string_builder struct. The string_builder will resize automatically. */
string_builder* string_builder_append_string(string_builder* sb, string* string);
string_builder* string_builder_append_int(string_builder* sb, int* integer);
string_builder* string_builder_append_chars(string_builder* sb, char* string);

/* Will clear everything in the string_builder resetting it, to a newly state. returns a cleared string_builder */
string_builder* string_builder_clear(string_builder* sb);

/* Cleans and destroys the string_builder along with all the data stored within.  */
void string_builder_destroy(string_builder* sb);

/* Inserts a string at the specified character position. the index denotes the character position.  */
string_builder* string_builder_insert_string(string_builder* sb, size_t index, string* string);
string_builder* string_builder_insert_int(string_builder* sb, size_t index, int* integer);
string_builder* string_builder_insert_chars(string_builder* sb, size_t index, char* string);

/* Removes the specified range of characters starting from the specified index position. */
string_builder* string_builder_remove(string_builder* sb, size_t index, size_t range);
/* removes a single character at the specified position */
string_builder* string_builder_remove_at(string_builder* sb, size_t index);

/* replaces a character at the specified position by the specified character. */
string_builder* string_builder_replace_char_at(string_builder* sb, size_t index, char* character);

/* returns a long string of all inserted/appended items in the string builder.  */
string* string_builder_to_string(string_builder* sb);
string* string_builder_copy_to_string(string_builder* sb);
char* string_builder_copy_to_char_array(string_builder* sb);
//======================= string_builder ==========================