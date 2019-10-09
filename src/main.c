#include "tree.h"
#include "weeder.h"
#include "pretty.h"
#include "type_checker.h"
#include <stdio.h>
#include "code_gen.h"
#include "falcon_arguments.h"
#include <stdlib.h>
#include "file_io.h"

char FALCON_DEBUG_PARSER;
char FALCON_DEBUG_AST;
char FALCON_DEBUG_CODEGEN;
char FALCON_DEBUG_TYPECHECK;
char FALCON_DEBUG_TYPECHECK_SYMBOLTABLE;

int lineno;

int GLOBAL_ERROR_COUNT;

int yyparse();

BODY* AST;

int main(int argc, char** argv)
{
	GLOBAL_ERROR_COUNT = 0;

	 //handles input arguments, just (somewhat) like gcc.
	falcon_config* config = falcon_argumenter(argc, argv);
	FALCON_DEBUG_PARSER = config->FALCON_DEBUG_PARSER;
	FALCON_DEBUG_AST = config->FALCON_DEBUG_AST;
	FALCON_DEBUG_TYPECHECK = config->FALCON_DEBUG_TYPECHECK;
	FALCON_DEBUG_TYPECHECK_SYMBOLTABLE = config->FALCON_DEBUG_TYPECHECK_SYMBOLTABLE;
	FALCON_DEBUG_CODEGEN = config->FALCON_DEBUG_CODEGEN;

	lineno = 1;
	GLOBAL_ERROR_COUNT += yyparse();

	if (GLOBAL_ERROR_COUNT > 0)
	{
		fprintf(stderr, "\x1b[37;1m");
		fprintf(stderr, "%d: ", lineno); //print  lineno
		fprintf(stderr, "\x1b[0m"); //reset color.
		fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
		fprintf(stderr, "%d Errors ", GLOBAL_ERROR_COUNT); //start print color magenta.
		fprintf(stderr, "\x1b[0m"); //reset color.
		fprintf(stderr, "Detected during scanning and parsing phase. Exiting..");
		fprintf(stderr, "\x1b[0m.\n");
		exit(1);
	}

	prettyAST(AST);

	weed_AST(AST);
	
	SymbolTable* root = type_checker(AST);
	
	if (GLOBAL_ERROR_COUNT > 0)
	{
		fprintf(stderr, "\x1b[37;1m");
		fprintf(stderr, "%d: ", lineno); //print  lineno
		fprintf(stderr, "\x1b[0m"); //reset color.
		fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
		fprintf(stderr, "%d Errors ", GLOBAL_ERROR_COUNT); //start print color magenta.
		fprintf(stderr, "\x1b[0m"); //reset color.
		fprintf(stderr, "Detected during type checking phase. Exiting..");
		fprintf(stderr, "\x1b[0m.\n");
		exit(1);
	}
	
	
	linked_list* IR_linked_list = linked_list_new();
	code_gen(AST, IR_linked_list, root);

	linked_list_print(&IR_linked_list);

	//GHJ: disabled for now, since it's quite bugged at the moment.
	//there's a bug with multiple expressions, 5+4 works fine but 5+4+3 is not handled correctly.
	//peephole (IR_linked_list);

	return 0;
}
