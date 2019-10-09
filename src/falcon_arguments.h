#pragma once
#include "array.h"

typedef struct falcon_config
{
	array* input_files;
	char FALCON_DEBUG_PARSER;
	char FALCON_DEBUG_TYPECHECK;
	char FALCON_DEBUG_TYPECHECK_SYMBOLTABLE;
	char FALCON_DEBUG_CODEGEN;
	char FALCON_DEBUG_AST;
} falcon_config;


falcon_config* falcon_argumenter(int argc, char** argv);
