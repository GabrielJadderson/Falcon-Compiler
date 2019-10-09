#include "falcon_arguments.h"
#include <string.h>
#include "memory.h"


falcon_config* falcon_argumenter(int argc, char** argv)
{
	falcon_config* config = Calloc(1, sizeof(falcon_config));

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-dts") == 0) //enable debugging for symbol tables in the type-checker.
		{
			config->FALCON_DEBUG_TYPECHECK_SYMBOLTABLE = 1;
		}
		if (strcmp(argv[i], "-dt") == 0) //enable debugging for type-checker.
		{
			config->FALCON_DEBUG_TYPECHECK = 1;
		}
		if (strcmp(argv[i], "-dc") == 0) //enable debugging for code-generation.
		{
			config->FALCON_DEBUG_CODEGEN = 1;
		}
		if (strcmp(argv[i], "-dp") == 0) //enable debugging for parser.
		{
			config->FALCON_DEBUG_PARSER = 1;
		}
		if (strcmp(argv[i], "-da") == 0) //enable debugging for parser.
		{
			config->FALCON_DEBUG_AST = 1;
		}
	}

	return config;
}
