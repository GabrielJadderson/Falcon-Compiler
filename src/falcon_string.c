#include "falcon_string.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "logger.h"

char* string_substring_from_left(char* array, size_t offset)
{
	size_t len = strlen(array);
	char* str_3 = Calloc(len - offset + 1, 1);
	strncpy(str_3, array, offset);
	return str_3;
}

char* string_substring_from_right(char* array, size_t offset)
{
	size_t len = strlen(array);
	char* str_3 = Calloc(len - offset + 1, 1);
	strncpy(str_3, array, len - offset);
	return str_3;
}


char *string_substring(char *string, size_t position, size_t length)
{
	char *str = Malloc(length + 1);
	size_t i;
	for (i = 0; i < length; i++)
	{
		*(str + i) = *((string + position - 1) + i);
	}
	//add null char at the end to terminate.
	*(str + i) = '\0';
	return str;
}

string* string_append_at(string* original, string* appendee, size_t index)
{
	//split the string in two. between the index.
	char* first = string_substring(original->Text, 0, index - 1);
	char* second = string_substring(original->Text, index, original->Length - index + 1);

	strcat(original->Text, first);
	strcat(original->Text, appendee->Text);
	strcat(original->Text, second);

	free(first);
	free(second);
	return original;
}

char* string_append_chars_at(char* original, char* appendee, size_t index)
{
	//split the string in two. between the index.
	char* first = string_substring(original, 0, index - 1);
	char* second = string_substring(original, index, strlen(original) - index + 1);

	strcat(original, first);
	strcat(original, appendee);
	strcat(original, second);

	free(first);
	free(second);
	return original;
}

char* string_int_to_string(int integer)
{
	char* buffer = Calloc(21, 1); //64-bit integer has max 20 characters + 1 for null.
	sprintf(buffer, "%d", integer);
	return buffer;
}


/**
 * constructs a new string by the size of the length specified.
 * substring.
 */
string string_new_by(char* text, size_t length)
{
	string str = { 0 };
	if (text)
	{
		str.Length = length;
		str.Text = strcpy(str.Text, text);
	}
	return str;
}


string string_new(char* text)
{
	string str = { 0 };
	if (text)
		str = string_new_by(text, strlen(text));
	return str;
}


string string_new_with_int(int integer)
{
	char* buffer = string_int_to_string(integer);
	string str = string_new(buffer);
	free(buffer);
	return str;
}


/**
 * for big strings!!
 */
string string_new_memcpy(char* text)
{
	string str = { 0 };
	if (text)
	{
		str.Length = strlen(text);
		str.Text = Malloc((str.Length + 1) * sizeof(char));
		memcpy(str.Text, text, str.Length + 1);
	}
	return str;
}


void string_delete(string * str)
{
	if (str)
	{
		if (str->Text)
			free(str->Text);
	}
}

/**
 * constructs a new string on the heap and copies the data into it.
 */
string* string_heap_new(char* text)
{
	if (text)
	{
		size_t len = strlen(text);
		return string_heap_new_by(text, len);
	}
	return NULL;
}

/**
 * constructs a new string on the heap and copies the data into it.
 */
string* string_heap_new_with_int(int integer)
{
	char* buffer = string_int_to_string(integer);
	string* final = string_heap_new(buffer);
	free(buffer);
	return final;
}


/**
 * constructs a new string by the size of the length specified.
 * substring.
 */
string* string_heap_new_by(char* text, size_t length)
{
	if (text)
	{
		string* str = Calloc(1, sizeof(string));
		if (str)
		{
			str->Length = length;
			str->Text = Calloc(str->Length + 1, sizeof(char));
			strcpy(str->Text, text); //copy the string
			return str;
		}
	}
	return NULL;
}

/*
 * creates a new string on the heap and returns the pointer to it.
 * the char* text is copied using memcpy.
 */
string* string_heap_new_memcpy(char* text)
{
	if (text)
	{
		string* str = Calloc(1, sizeof(string));
		if (str)
		{
			str->Length = strlen(text);
			str->Text = Malloc((str->Length + 1) * sizeof(char));
			memcpy(str->Text, text, str->Length + 1);
			return str;
		}
	}
	return NULL;
}

/**
 * Very fast for already allocated data.
 */
string* string_heap_new_allocated(char** text, size_t length)
{
	if (text)
	{
		string* str = Calloc(1, sizeof(string));
		if (str)
		{
			str->Length = length;
			str->Text = *text;
			return str;
		}
	}
	return NULL;
}

/**
 * deletes the string struct and deletes the data.
 */
void string_heap_delete(string * str)
{
	if (str)
	{
		if (str->Text)
			free(str->Text);
		free(str);
	}
}

void string_heap_delete_struct(string * str)
{
	if (str)
	{
		free(str);
	}
}

void string_heap_delete_allocated(string * str)
{
	if (str)
	{
		if (str->Text)
			free(str->Text);
		free(str);
	}
}


string* string_concatenate_from_chars(char* stringA, char* stringB)
{
	if (!stringA || !stringB) return NULL;
	size_t length_a = strlen(stringA);
	size_t length_b = strlen(stringB);
	size_t total_length = length_a + length_b;
	char* ptr = Calloc(total_length + 1, sizeof(char)); // +1 for the null character.
	if (!ptr) { printf("FAILED TO ALLOCATE SPACE FOR STRING CONCATENATION\n"); exit(1); }
	strcpy(ptr, stringA); //first we copy the contents of a into the new space
	strcat(ptr, stringB); //then we use strcat to append b after we've added a.

	return string_heap_new(ptr);
}

string* string_concatenate(string* stringA, string* stringB)
{
	if (!stringA || !stringB) return NULL;

	size_t length_a = stringA->Length;
	size_t length_b = stringB->Length;
	size_t total_length = length_a + length_b;
	char* ptr = Calloc(total_length + 1, sizeof(char)); // +1 for the null character.
	if (!ptr) { printf("FAILED TO ALLOCATE SPACE FOR STRING CONCATENATION\n"); exit(1); }
	strcpy(ptr, stringA->Text); //first we copy the contents of a into the new space
	strcat(ptr, stringB->Text); //then we use strcat to append b after we've added a.

	return string_heap_new(ptr);
}

char* string_concat(char* stringA, char* stringB)
{
	if (!stringA || !stringB) return NULL;
	size_t length_a = strlen(stringA);
	size_t length_b = strlen(stringB);
	size_t total_length = length_a + length_b;
	char* ptr = Calloc(total_length + 1, sizeof(char)); // +1 for the null character.
	if (!ptr) { printf("FAILED TO ALLOCATE SPACE FOR STRING CONCATENATION\n"); exit(1); }
	strcpy(ptr, stringA); //first we copy the contents of a into the new space
	strcat(ptr, stringB); //then we use strcat to append b after we've added a.
	//free both a and b, since we allocate them in flex during the scanning phase.
	return ptr;
}

char* string_concatenate_to_chars(string* stringA, string* stringB)
{
	if (!stringA || !stringB) return NULL;

	size_t length_a = stringA->Length;
	size_t length_b = stringB->Length;
	size_t total_length = length_a + length_b;
	char* ptr = Calloc(total_length + 1, sizeof(char)); // +1 for the null character.
	if (!ptr) { printf("FAILED TO ALLOCATE SPACE FOR STRING CONCATENATION\n"); exit(1); }
	strcpy(ptr, stringA->Text); //first we copy the contents of a into the new space
	strcat(ptr, stringB->Text); //then we use strcat to append b after we've added a.

	return string_heap_new(ptr)->Text;
}



/**
 * prints string
 */
void string_print(string * str)
{
	if (str)
		printf("%s\n", str->Text);
}





//======================= string_builder ==========================
//Initializes a new instance of the string_builder
string_builder* string_builder_new()
{
	string_builder* sb = Calloc(1, sizeof(string_builder));
	sb->str = string_heap_new("");
	return sb;
}
//Initializes a new instance of the string_builder using the specified string.
string_builder* string_builder_new_with_string(string* string)
{
	string_builder* sb = Calloc(1, sizeof(string_builder));
	if (string)
		sb->str = string_heap_new(string->Text);
	else
		sb->str = string_heap_new("");
	return sb;
}

/* Appends a copy of the specified string to the specified string_builder struct.
 * returns the same string_builder struct. The string_builder will resize automatically. */
string_builder* string_builder_append_string(string_builder* sb, string* string)
{
	if (!sb || !string) return NULL;
	sb->str->Text = string_concatenate_to_chars(sb->str, string);
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}
string_builder* string_builder_append_int(string_builder* sb, int* integer)
{
	if (!sb || !integer) return NULL;
	char* buffer = string_int_to_string(*integer);
	string* s = string_heap_new_by(buffer, 65);
	sb->str->Text = string_concatenate_to_chars(sb->str, s);
	free(buffer);
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}

string_builder* string_builder_append_chars(string_builder* sb, char* string)
{
	if (!sb || !string) return NULL;
	sb->str->Text = string_concat(sb->str->Text, string);
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}

/* Will clear everything in the string_builder resetting it, to a newly state. returns a cleared string_builder */
string_builder* string_builder_clear(string_builder* sb)
{
	if (!sb) return NULL;
	string_delete(sb->str);
	sb->str = string_heap_new("");
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}

/* Cleans and destroys the string_builder along with all the data stored within.  */
void string_builder_destroy(string_builder* sb)
{
	if (!sb) return;
	string_delete(sb->str);
	if (sb != 0)
		free(sb);
	sb = 0;
}

/* Inserts a string at the specified character position. */
string_builder* string_builder_insert_string(string_builder* sb, size_t index, string* string)
{
	if (!sb || !string) return NULL;
	sb->str = string_append_at(sb->str, string, index);
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}


string_builder* string_builder_insert_int(string_builder* sb, size_t index, int* integer)
{
	if (!sb || !integer) return NULL;
	char* buffer = string_int_to_string(*integer);
	string* s = string_heap_new_by(buffer, 65);
	sb->str = string_append_at(sb->str, s, index);
	free(buffer);
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}

string_builder* string_builder_insert_chars(string_builder* sb, size_t index, char* string)
{
	if (!sb || !string) return NULL;
	string_append_chars_at(sb->str->Text, string, index);
	sb->str->Length = strlen(sb->str->Text);
	return sb;
}

/* Removes the specified range of characters starting from the specified index position. */
string_builder* string_builder_remove(string_builder* sb, size_t index, size_t range)
{
	if (!sb || range == 0) return NULL;
	//split the string in two. between the index.
	char* first = string_substring(sb->str->Text, 0, index - 1);
	char* second = string_substring(sb->str->Text, index + range, sb->str->Length - index + 1);
	//add first
	char* new_str = string_concat(sb->str->Text, first);
	free(first);
	string_heap_delete(sb->str);

	//add second
	string* final_str = string_concatenate_from_chars(new_str, second);
	free(second);
	free(new_str);

	sb->str = final_str;
	return sb;
}
/* removes a single character at the specified position */
string_builder* string_builder_remove_at(string_builder* sb, size_t index)
{
	if (!sb) return NULL;

	//split the string in two. between the index.
	char* first = string_substring(sb->str->Text, 0, index - 1);
	char* second = string_substring(sb->str->Text, index + 1, sb->str->Length - index + 1);

	//add first
	char* new_str = string_concat(sb->str->Text, first);
	free(first);
	string_heap_delete(sb->str);

	//add second
	string* final_str = string_concatenate_from_chars(new_str, second);
	free(second);
	free(new_str);

	sb->str = final_str;
	return sb;
}

/* replaces a character at the specified position by the specified character. */
string_builder* string_builder_replace_char_at(string_builder* sb, size_t index, char* character)
{
	if (!sb || !character) return NULL;

	//split the string in two. between the index.
	char* first = string_substring(sb->str->Text, 0, index - 1);
	char* second = string_substring(sb->str->Text, index + 1, sb->str->Length - index + 1);


	//add first
	char* new_str = string_concat(sb->str->Text, first);
	free(first);
	string_heap_delete(sb->str);

	char* char_str = string_concat(new_str, character);
	free(new_str);

	//add second
	string* final_str = string_concatenate_from_chars(char_str, second);
	free(second);
	free(char_str);

	sb->str = final_str;
	return sb;
}

/* returns a long string of all inserted/appended items in the string builder.  */
string* string_builder_to_string(string_builder* sb)
{
	if (!sb) return NULL;
	sb->str->Length = strlen(sb->str->Text);
	return sb->str;
}

/* returns a long string of all inserted/appended items in the string builder.  */
string* string_builder_copy_to_string(string_builder* sb)
{
	if (!sb) return NULL;
	sb->str->Length = strlen(sb->str->Text);
	string* out = string_heap_new_by(sb->str->Text, sb->str->Length);
	return out;
}

/* returns a long string of all inserted/appended items in the string builder.  */
char* string_builder_copy_to_char_array(string_builder* sb)
{
	if (!sb) return NULL;
	string* str = string_builder_copy_to_string(sb);
	return str->Text;
}

//======================= string_builder ==========================