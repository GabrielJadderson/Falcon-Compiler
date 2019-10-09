#pragma once
#include "falcon_string.h"
#include <stdio.h>
#include "linked_list.h"

typedef struct falcon_file_handle
{
	string* file_path;
	string* file_data;
	FILE* file_handle;
	size_t file_size;
	bool file_is_opened;
} falcon_file_handle;

falcon_file_handle* file_open(char* filepath);
bool file_close(falcon_file_handle* fh);
bool file_close_and_clean(falcon_file_handle* fh);


void file_write_linked_list(char* filepath, linked_list** list);
void file_write_string(char* filepath, char* string);
