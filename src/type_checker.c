#include "type_checker.h"
#include <stdio.h>
#include "symbol.h"
#include "memory.h"
#include "array.h"
#include "logger.h"

extern char FALCON_DEBUG_TYPECHECK;
extern char FALCON_DEBUG_TYPECHECK_SYMBOLTABLE;
extern int GLOBAL_ERROR_COUNT;

enum { type_phase_collectK, type_phase_calculateK, type_phase_verifyK } phase;
size_t GLOBAL_ACT_LIST_SIZE;
int GLOBAL_NESTING_DEPTH;

SymbolTable* type_checker(BODY* AST)
{
	GLOBAL_NESTING_DEPTH = 0;
	GLOBAL_ACT_LIST_SIZE = 0;
	//create root scope
	SymbolTable* root = initSymbolTable();

	//1. Collect: store, root and initiate collection phase.
	phase = type_phase_collectK;
	type_check_BODY(AST, root);
	if (FALCON_DEBUG_TYPECHECK)
		printf("=========================== [Type-checker] Finished collection phase.\n");


		//2. calculation phase.
	phase = type_phase_calculateK;
	type_check_BODY(AST, root);
	if (FALCON_DEBUG_TYPECHECK)
		printf("=========================== [Type-checker] Finished calculation phase.\n");

		//3. verify.
	phase = type_phase_verifyK;
	type_check_BODY(AST, root);
	if (FALCON_DEBUG_TYPECHECK)
		printf("=========================== [Type-checker] Finished verification phase.\n");


	if (FALCON_DEBUG_TYPECHECK_SYMBOLTABLE)
	{
		printf("Scope of main:\n");
		dumpSymbolTable(root);
		printf("---------------------\n");
	}

	if (FALCON_DEBUG_TYPECHECK || FALCON_DEBUG_TYPECHECK_SYMBOLTABLE)
	{
		printf("THE TYPE-CHECKER HAS PASSED! GND: %d\n", GLOBAL_NESTING_DEPTH);
	}

	return root;
}


static void type_checker_print_type(type_info* ti)
{
	if (ti && FALCON_DEBUG_TYPECHECK)
	{
		switch (ti->kind)
		{
			case t_idk:
			{
				printf("%s", ti->type_id);
			}
			break;
			case t_intK:
			{
				printf("int");
			} break;
			case t_boolK:
			{
				printf("bool");
			} break;
			case t_stringK:
			{
				printf("string");
			} break;
			case t_recordK:
			{
				printf("record of {");
				size_t length = ti->record_var_decl_list_types->length;
				for (size_t i = 0; i < length; i++)
				{
					type_info* stored_ti = (type_info*)array_at(ti->record_var_decl_list_types, i);
					if (stored_ti->kind != t_recordK)
						type_checker_print_type(stored_ti);
					printf(",");
				}
				printf("};");

			}
			break;
			case t_arrayK:
			{
				printf("array of ");
				type_checker_print_type(ti->array_type_info);
			}
			break;
			case t_nullK:
			{
				printf("null");
			}
			break;
			case t_typeK:
			{
				printf("%s", ti->type_id);
			}
			break;
			default:
			{
				printf("INVALID_TYPE");
			}
			break;
		}
	}
}
static void type_checker_print_type_error(type_info * ti)
{
	if (ti)
	{
		switch (ti->kind)
		{
			case t_idk:
			{
				fprintf(stderr, "%s", ti->type_id);
			}
			break;
			case t_intK:
			{
				fprintf(stderr, "int");
			} break;
			case t_boolK:
			{
				fprintf(stderr, "bool");
			} break;
			case t_stringK:
			{
				fprintf(stderr, "string");
			} break;
			case t_recordK:
			{
				fprintf(stderr, "record of {");
				size_t length = ti->record_var_decl_list_types->length;
				for (size_t i = 0; i < length; i++)
				{
					type_info* stored_ti = (type_info*)array_at(ti->record_var_decl_list_types, i);
					fprintf(stderr, "%s:", stored_ti->symbol->name);
					if (stored_ti->kind != t_recordK)
						type_checker_print_type_error(stored_ti);
					fprintf(stderr, ",");
				}
				fprintf(stderr, "};");
			}
			break;
			case t_arrayK:
			{
				fprintf(stderr, "array of ");
				type_checker_print_type_error(ti->array_type_info);
			}
			break;
			case t_nullK:
			{
				fprintf(stderr, "null");
			}
			break;
			case t_typeK:
			{
				fprintf(stderr, "%s", ti->type_id);
			}
			break;
			default:
			{
				fprintf(stderr, "INVALID_TYPE");
			}
			break;
		}
	}
}


static void type_checker_print_stm(int lineno, type_info * ti, char* type)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "%s-statement has illegal expression type of ", type);
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}


static void type_checker_print_template(int lineno, type_info * a, type_info* b, char* format)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "%s ", format);
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(a);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " and ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(b);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_undefined_variable(int lineno, char* id)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, " Type ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%s", id);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " is not defined");
	fprintf(stderr, "\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_function_type_mismatch(int lineno, char* name, type_info* ti_head, type_info* body)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, " In function ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%s", name);
	fprintf(stderr, "'\x1b[0m:");
	fprintf(stderr, " function return type mismatch - head type is ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti_head);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, ", body return type ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(body);
	fprintf(stderr, "'\x1b[0m:");
	fprintf(stderr, "\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_function_no_return_type(int lineno, char* name, type_info* ti_head)
{
	printf("\x1b[37;1m");
	printf("%d: ", lineno); //print  lineno
	printf("\x1b[0m"); //reset color.
	printf("\x1b[36;1m"); //start print color magenta.
	printf("Warning: "); //start print color magenta.
	printf("\x1b[0m"); //reset color.
	printf(" In function ");
	printf("\x1b[37;1m'");
	printf("%s", name);
	printf("'\x1b[0m:");
	printf(" function missing return type, expected ");
	printf("\x1b[37;1m'");
	type_checker_print_type(ti_head);
	printf("'\x1b[0m");
	printf(" found nothing");
	printf("\x1b[0m.\n");
}

static void type_checker_print_term_function_call_non_function(int lineno, type_info* function)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Illegal action, trying to call the non-function: ");
	if (function->func_id)
		fprintf(stderr, "\x1b[37;1m'%s'\x1b[0m.\n", function->func_id);
	GLOBAL_ERROR_COUNT++;
}

/* //we need to look into this in the future.
static void type_checker_print_variable_access_illegal_type(int lineno, type_info* var_recursive)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Illegal action, only records are allowed to be accessed, the type was ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(var_recursive);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}
*/

static void type_checker_print_variable_array_access_illegal_indexing_method(int lineno, type_info* exp_recursive)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Only integers are allowed to be used as indexing type. Instead the type was ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(exp_recursive);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

/* GHJ: we have decided to allow this functionality.
static void type_checker_print_function_return_main_scope(int lineno)
{
	printf("\x1b[37;1m");
	printf("%d: ", lineno); //print  lineno
	printf("\x1b[0m"); //reset color.
	printf("\x1b[36;1m"); //start print color magenta.
	printf("Warning: "); //start print color magenta.
	printf("\x1b[0m"); //reset color.
	printf("Returning from main scope");
	printf("\x1b[0m.\n");
}
*/

static void type_checker_print_function_call_invalid_parameter_count(int lineno, size_t length_of_func_parameters, size_t length_of_call_arguments, char* name)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Calling the function ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%s", name);
	fprintf(stderr, "'\x1b[0m with");
	if (length_of_call_arguments < length_of_func_parameters)
		fprintf(stderr, " too few ");
	else if (length_of_call_arguments > length_of_func_parameters)
		fprintf(stderr, " too many ");
	fprintf(stderr, "arguments\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_function_call_invalid_parameter_types(int lineno, type_info* ti_func, type_info*  act_list)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Conflicting types for function ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%s", ti_func->func_id);
	fprintf(stderr, "'\x1b[0m expected \x1b[37;1m(");
	size_t parameter_count = ti_func->function_var_decl_list_types->length;
	for (size_t i = 0; i < parameter_count; ++i)
	{
		type_info* parameter = array_at(ti_func->function_var_decl_list_types, i);
		type_checker_print_type_error(parameter);
		if ((i + 1) != parameter_count) fprintf(stderr, ", ");
	}
	fprintf(stderr, ")\x1b[0m, instead the function was called with \x1b[37;1m(");
	size_t argument_count = act_list->act_list_types->length;
	for (size_t i = 0; i < argument_count; ++i)
	{
		type_info* argument = array_at(act_list->act_list_types, i);
		type_checker_print_type_error(argument);
		if ((i + 1) != parameter_count) fprintf(stderr, ", ");
	}
	fprintf(stderr, ")\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_stm_assign(int lineno, type_info * a, type_info * b)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Illegal assignment type for ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(a);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " and \x1b[37;1m'");
	type_checker_print_type_error(b);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_stm_assign_function_not_supported(int lineno)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Illegal action, a function cannot be assigned to.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_stm_assign_to_type(int lineno, type_info * ti_exp, char* id)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Cannot assign ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%s", id);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " to ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti_exp);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " because ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%s", id);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " is a type.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_stm_assign_array_mismatch_types(int lineno, type_info* A, type_info* B)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Cannot assign an array of type ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(A);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " to another of type \x1b[37;1m'");
	type_checker_print_type_error(B);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}
static void type_checker_print_variable_array_access_non_array_type(int lineno, type_info* var_recursive)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Illegal action, trying to access ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(var_recursive);
	fprintf(stderr, "'\x1b[0m as an array.\n");
	GLOBAL_ERROR_COUNT++;
}

static void type_checker_print_stm_assign_record_mismatch_elements(int lineno, type_info* ti_element_in_var, type_info* ti_element_in_exp, int i)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Assigning two records with different types at record index ");
	fprintf(stderr, "\x1b[37;1m'");
	fprintf(stderr, "%d", i);
	fprintf(stderr, "'\x1b[0m with");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti_element_in_var);
	fprintf(stderr, "'\x1b[0m");
	fprintf(stderr, " to ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti_element_in_exp);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}
static void type_checker_print_stm_assign_record_mismatch_count(int lineno, array* A, array* B)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Assigning two records with different sizes of ");
	fprintf(stderr, "'\x1b[0m with \x1b[37;1m(");
	size_t len_a = A->length;
	for (size_t i = 0; i < len_a; ++i)
	{
		type_info* parameter = array_at(A, i);
		type_checker_print_type_error(parameter);
		if ((i + 1) != len_a) fprintf(stderr, ", ");
	}
	fprintf(stderr, ")\x1b[0m to \x1b[37;1m(");
	size_t len_b = B->length;
	for (size_t i = 0; i < len_b; ++i)
	{
		type_info* argument = array_at(B, i);
		type_checker_print_type_error(argument);
		if ((i + 1) != len_b) fprintf(stderr, ", ");
	}
	fprintf(stderr, ")\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}


static void type_checker_print_abs(int lineno, type_info * ti)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Absolute of ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti);
	fprintf(stderr, "'\x1b[0m is illegal.\n");
	GLOBAL_ERROR_COUNT++;
}


static void type_checker_print_negation(int lineno, type_info * ti)
{
	fprintf(stderr, "\x1b[37;1m");
	fprintf(stderr, "%d: ", lineno); //print  lineno
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
	fprintf(stderr, "Error: "); //start print color magenta.
	fprintf(stderr, "\x1b[0m"); //reset color.
	fprintf(stderr, "Cannot negate ");
	fprintf(stderr, "\x1b[37;1m'");
	type_checker_print_type_error(ti);
	fprintf(stderr, "'\x1b[0m.\n");
	GLOBAL_ERROR_COUNT++;
}
static type_info* type_check_recursively_lookup_id(char* id, SymbolTable* scope, int lineno)
{
	if (id)
	{
		//printf("entered id lookup for %s\n", id);
		SYMBOL* symbol = getSymbol(scope, id);
		//dumpSymbolTable(scope);
		if (symbol != NULL)
		{
			type_info* ti = symbol->value;
			if (ti)
			{
				//printf("found symbol of type: %d, type_id:%s\n", ti->kind, ti->type_id);
				if (ti->kind == t_idk)
				{
					if (ti->type_info_child)
					{
						if (ti->type_info_child->type_id)
						{
							return type_check_recursively_lookup_id(ti->type_info_child->type_id, scope, lineno);
						}
						else
						{
							return ti->type_info_child;
						}
					}
					else
					{
						return type_check_recursively_lookup_id(ti->type_id, scope, lineno);
						//return ti;
					}
				}
				else if (ti->kind == t_typeK)
				{
					if (ti->type_info_child)
					{
						if (ti->type_info_child->type_id)
						{
							return type_check_recursively_lookup_id(ti->type_info_child->type_id, scope, lineno);
						}
						else
						{
							return ti->type_info_child;
						}
					}
					else
					{
						log_internal_error(__FILE__, __func__, __LINE__, "type declaration must have a valid type_info_child. possible failure in structual buildup during collection phase.");
					}

				}
				else
				{
					return ti;
				}
			}
			else type_checker_print_undefined_variable(lineno, id);
		}
		else type_checker_print_undefined_variable(lineno, id);
	}
	return NULL;
}


static type_info* type_check_recursively_lookup_type_info(type_info* ti, SymbolTable* scope, int lineno)
{
	if (ti)
	{
		if (ti->type_id)
		{
			return type_check_recursively_lookup_id(ti->type_id, scope, lineno);
		}
		else return ti;
	}
	else { log_internal_error(__FILE__, __func__, __LINE__, "type info was null"); }
	return NULL;
}

/*
static void type_checker_print_debug_ti(type_info* ti, char* str, int lineno)
{
	if (ti)
	{
		//printf("line: %d -> %s: kind: %s, id: %s, type_id: %s, func_id: %s, is_a_type: %d, is_a_variable: %d, is_a_function: %d\n",
		//	lineno, str, type_checker_kind_enum_to_string(ti), ti->id,
		//	ti->type_id, ti->func_id, ti->is_a_type, ti->is_a_variable, ti->is_a_function);
		printf("line: %d -> %s: kind: %d, type_id: %s, func_id: %s, is_a_type: %d, is_a_variable: %d, is_a_function: %d\n",
			lineno, str, ti->kind, ti->type_id,
			ti->func_id, ti->is_a_type, ti->is_a_variable, ti->is_a_function);
	}
	else
	{
		log_internal_error(__FILE__, __func__, __LINE__, "type info was null");
	}
}
*/



/* takes in a type_info struct and creates a new type_info struct and copies all fields from the host to the target. */
static type_info* type_check_create_copy_of_type_info(type_info* master)
{
	if (master)
	{
		type_info* out = Calloc(1, sizeof(type_info));
		out->kind = master->kind;
		out->nesting_depth = master->nesting_depth;
		out->record_nesting_depth = master->record_nesting_depth;
		out->function_nesting_depth = master->function_nesting_depth;
		out->func_id = master->func_id;
		out->type_id = master->type_id;
		out->is_record = master->is_record;
		out->has_return = master->has_return;
		out->is_a_type = master->is_a_type;
		out->is_a_function = master->is_a_function;
		out->is_a_variable = master->is_a_variable;
		out->is_a_record = master->is_a_record;
		out->is_record_element = master->is_record_element;
		out->is_verified = master->is_verified;
		out->array_type_info = master->array_type_info;
		out->type_info_child = master->type_info_child;
		out->record_var_decl_list = master->record_var_decl_list;
		out->function_var_decl_list_types = master->function_var_decl_list_types;
		out->record_var_decl_list_types = master->record_var_decl_list_types;
		out->act_list_types = master->act_list_types;
		out->stm_list_types = master->stm_list_types;
		out->record_scope = master->record_scope;
		out->sym_table = master->sym_table;
		out->symbol = master->symbol;
		out->act_list_count = master->act_list_count;
		return out;
	}
	else log_internal_error(__FILE__, __func__, __LINE__, "FAILED TO CREATE A COPY OF THE TYPE.");
	return NULL;
}


static type_info* type_checker_lookup_once(char* str, SymbolTable* scope, int lineno)
{
	if (str)
	{
		SYMBOL* symbol = getSymbol(scope, str);
		if (symbol)
		{
			type_info* ti = symbol->value;
			if (ti)
			{
				return ti;
			}
			else type_checker_print_undefined_variable(lineno, str);
		}
		else type_checker_print_undefined_variable(lineno, str);
	}
	else
	{
		log_internal_error(__FILE__, __func__, __LINE__, "string was null");
		printf("%d\n", lineno);
	}
	return NULL;
}

//just does a check on type_id, type_id is null then it is a primitive type, if not then lookup once from the above.
static type_info* type_check_lookup_once_type_info(type_info* ti, SymbolTable* scope, int lineno)
{
	if (ti)
	{
		if (ti->type_id)
		{
			return type_checker_lookup_once(ti->type_id, scope, lineno);
		}
		else return ti;
	}
	else { log_internal_error(__FILE__, __func__, __LINE__, "type info was null"); }
	return NULL;
}

type_info* type_check_STRING(STRING * s, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <string> %d -- %d\n", 0, phase);
	switch (s->kind)
	{
		case stringK:
		{
			if (FALCON_DEBUG_TYPECHECK)
				printf("%d <string> -> \"%s\"\n", s->lineno, s->str);
			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_stringK;
			ti->sym_table = scope;
			s->type = ti;
		}
		break;
		case string_concatenationK:
		{
			if (FALCON_DEBUG_TYPECHECK)
				printf("%d <string> -> \"%s\"\n", s->lineno, s->str);
			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_stringK;
			ti->sym_table = scope;
			s->type = ti;
		}
		break;
	}
	return s->type;
}

type_info* type_check_FUNCTION(FUNCTION * f, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <function> %d -- %d\n", 0, phase);

	if (phase == type_phase_collectK)
	{
		SymbolTable* func_scope = scopeSymbolTable(scope);
		f->scope = func_scope;
		GLOBAL_NESTING_DEPTH++;
	}

	type_info* ti_head = type_check_HEAD(f->func_head, f->scope);

	type_info* body = type_check_BODY(f->func_body, f->scope);

	type_check_TAIL(f->func_tail, f->scope);

	if (phase == type_phase_collectK)
	{
		f->type = ti_head;
		f->type->is_a_function = 1;
		f->type->is_a_type = 0;
		f->type->is_a_variable = 0;
	}

	if (FALCON_DEBUG_TYPECHECK_SYMBOLTABLE) //debug the symbol table during the collection phase in each function.
	{
		if (phase == type_phase_collectK)
		{
			printf("COLLECTION PHASE: Scope of function %s:\n", f->func_head->function_id);
			dumpSymbolTable(f->scope);
			printf("---------------------\n");
		}
	}


	if (phase == type_phase_calculateK || phase == type_phase_verifyK)
	{

		if (FALCON_DEBUG_TYPECHECK_SYMBOLTABLE) //debug the symbol table during the verification phase in each function.
		{
			if (phase == type_phase_verifyK)
			{
				printf("VERIFY PHASE: Scope of function %s:\n", f->func_head->function_id);
				dumpSymbolTable(f->scope);
				printf("---------------------\n");
			}
		}

		type_info* ti_recursive_lookup = type_check_recursively_lookup_id(f->func_head->function_id, f->scope->next, f->lineno);
		if (ti_recursive_lookup)
		{
			f->type = ti_recursive_lookup;
		}



		if (body)
		{
			if (body->return_type) //found a valid return statement in body.
			{
				if (ti_recursive_lookup)
				{
					if (ti_recursive_lookup->kind == body->return_type->kind)
					{
					}
					else
					{
						type_checker_print_function_type_mismatch(f->lineno, f->func_head->function_id, ti_recursive_lookup, body->return_type);
					}
				}
				else
				{
					log_internal_error(__FILE__, __func__, __LINE__, "Failed to retrieve function type from symbol table.");
				}
			}
			else //no return type for the function found.
			{
				type_checker_print_function_no_return_type(f->lineno, f->func_head->function_id, ti_recursive_lookup);
			}
		}
		else
		{
			type_checker_print_function_no_return_type(f->lineno, f->func_head->function_id, ti_recursive_lookup);
		}
	}

	return f->type;
}

type_info* type_check_HEAD(HEAD * head, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <head> %d -- %d\n", 0, phase);

	type_info* type = type_check_TYPE(head->return_type, scope->next);
	type_info* ti_par_decl_list = type_check_PAR_DECL_LIST(head->params, scope);

	if (phase == type_phase_collectK)
	{

		//we want to create a copy of the type. and then assign our own data to it.
		type_info* function_type = type_check_create_copy_of_type_info(type);
		function_type->type_id = head->function_id;
		function_type->func_id = head->function_id;
		function_type->sym_table = scope;
		function_type->is_a_variable = 0;
		function_type->is_a_function = 1;
		function_type->is_a_type = 0;
		function_type->is_verified = 0;
		function_type->type_info_child = type; //our child is <type>
		if (ti_par_decl_list)
			function_type->function_var_decl_list_types = ti_par_decl_list->function_var_decl_list_types;
		head->type = function_type;
		head->type->symbol = putSymbol(scope->next, head->function_id, head->type);
		//dumpSymbolTable(scope);

	}


	if (phase == type_phase_calculateK || phase == type_phase_verifyK)
	{
		type_info* type_info_self = type_checker_lookup_once(head->function_id, scope->next, head->lineno);
		if (type_info_self)
		{
			//type_checker_print_debug_ti(type_info_self, "head type_info self", head->lineno);
			if (!type_info_self->is_verified)
			{
				if (type_info_self->type_info_child)
				{
					type_info* type_info_child_recursive = type_check_recursively_lookup_id(type_info_self->type_info_child->type_id, scope->next, head->lineno);
					if (type_info_child_recursive)
					{

						type_info* function_type = type_check_create_copy_of_type_info(type_info_child_recursive);
						function_type->is_verified = 1;
						function_type->type_id = head->function_id;
						function_type->func_id = head->function_id;
						function_type->is_a_variable = 0;
						function_type->is_a_function = 1;
						function_type->is_a_type = 0;
						if (ti_par_decl_list)
							function_type->function_var_decl_list_types = ti_par_decl_list->function_var_decl_list_types;
						head->type = function_type;
						head->type->symbol = putSymbol(scope->next, head->function_id, head->type);
					}
					else
					{
						type_info* function_type = type_check_create_copy_of_type_info(type_info_self->type_info_child);
						function_type->is_verified = 1;
						function_type->type_id = head->function_id;
						function_type->func_id = head->function_id;
						function_type->is_a_variable = 0;
						function_type->is_a_function = 1;
						function_type->is_a_type = 0;
						if (ti_par_decl_list)
							function_type->function_var_decl_list_types = ti_par_decl_list->function_var_decl_list_types;
						head->type = function_type;
						head->type->symbol = putSymbol(scope->next, head->function_id, head->type);
					}
				}
				else
				{
					type_info* function_type = type_check_create_copy_of_type_info(type_info_self);
					function_type->is_verified = 1;
					function_type->type_id = head->function_id;
					function_type->func_id = head->function_id;
					function_type->is_a_variable = 0;
					function_type->is_a_function = 1;
					function_type->is_a_type = 0;
					if (ti_par_decl_list)
						function_type->function_var_decl_list_types = ti_par_decl_list->function_var_decl_list_types;
					head->type = function_type;
					head->type->symbol = putSymbol(scope->next, head->function_id, head->type);
				}
			}
		}
		else log_internal_error(__FILE__, __func__, __LINE__, "error in <var_type> failed finding our self in the symbol table.");

	}

	return head->type;
}

type_info* type_check_TAIL(TAIL * tail, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <tail> %d -- %d \n", 0, phase);

	if (phase == type_phase_collectK)
	{
		if (tail || scope)
		{
			//no-op. to prevent gcc compilation warnings.
		}


		GLOBAL_NESTING_DEPTH--;
	}
	return NULL;
}

type_info* type_check_TYPE(TYPE * type, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <type> %d -- %d\n", type->kind, phase);
	switch (type->kind)
	{
		case type_idK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_idk;
				ti->type_id = type->val.type_id;
				ti->sym_table = scope;
				type->ti = ti;
			}

		}
		break;
		case type_intK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_intK;
				ti->sym_table = scope;
				type->ti = ti;
			}
		}
		break;
		case type_boolK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_boolK;
				ti->sym_table = scope;
				type->ti = ti;
			}

		}
		break;
		case type_arrayK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_arrayK;
				ti->array_type_info = type_check_TYPE(type->val.type, scope);
				ti->sym_table = scope;
				type->ti = ti;
			}

			else if (phase == type_phase_calculateK)
			{
				type_check_TYPE(type->val.type, scope);
			}
			else if (phase == type_phase_verifyK)
			{
				type_check_TYPE(type->val.type, scope);
			}

		}
		break;
		case type_recordK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_recordK;

				SymbolTable* st = scopeSymbolTable(scope);

				//store the decl list for the record.
				ti->record_var_decl_list = type->val.var_decl_list;
				array* arr = array_new();
				type_check_VAR_DECL_LIST(type->val.var_decl_list, st, arr);
				if (FALCON_DEBUG_TYPECHECK)
				{
					printf("%d <type> -> record of {", type->lineno);
					size_t length = arr->length;
					for (size_t i = 0; i < length; i++)
					{
						type_info* tii = (type_info*)array_at(arr, i);
						tii->is_record_element = 1;
						type_checker_print_type(tii);
						printf(",");
					}
					printf("};\n");
				}
				ti->record_scope = st;
				ti->sym_table = scope;
				ti->record_var_decl_list_types = arr;
				ti->is_record = 1;
				type->ti = ti;
			}
			if (phase == type_phase_verifyK || phase == type_phase_calculateK)
			{
				//store the decl list for the record.
				array* arr = array_new();
				type_check_VAR_DECL_LIST(type->val.var_decl_list, type->ti->record_scope, arr);
				type->ti->record_var_decl_list_types = arr;
			}

		}
		break;
		case type_stringK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_stringK;
				ti->sym_table = scope;
				type->ti = ti;
			}
		}
		break;

		default:  break;
	}
	return type->ti;
}


type_info* type_check_PAR_DECL_LIST(PAR_DECL_LIST * par_decl_list, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <par_decl_list> %d -- %d\n", par_decl_list->kind, phase);
	switch (par_decl_list->kind)
	{
		case par_decl_list_var_decl_listK:
		{
			if (phase == type_phase_collectK)
			{
				array* arr = array_new();
				type_info* ti = type_check_VAR_DECL_LIST(par_decl_list->val.var_decl_list, scope, arr);
				ti->function_var_decl_list_types = arr;
				par_decl_list->type = ti;
			}
			if (phase == type_phase_calculateK || phase == type_phase_verifyK) //TODO CHANGE ALL OF THESE CHECKS TO INCLUDE THE CALCULATION PHASE phase == type_phase_verifyK || phase == type_phase_calculateK
			{
				array* arr = array_new();
				type_info* ti = type_check_VAR_DECL_LIST(par_decl_list->val.var_decl_list, scope, arr);
				ti->function_var_decl_list_types = arr;
				par_decl_list->type = ti;
			}
		}
		break;
		case par_decl_list_emptyK:
		{

		}
		break;

		default:  break;
	}
	return par_decl_list->type;
}


type_info* type_check_VAR_DECL_LIST(VAR_DECL_LIST * var_decl_list, SymbolTable * scope, array * arr)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <var_decl_list> %d -- %d\n", var_decl_list->kind, phase);
	switch (var_decl_list->kind)
	{
		case var_decl_list_pairK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = type_check_VAR_TYPE(var_decl_list->val.pair.head, scope);
				var_decl_list->type = ti;
				array_push(arr, ti);
				type_check_VAR_DECL_LIST(var_decl_list->val.pair.tail, scope, arr);
			}
			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* ti = type_check_VAR_TYPE(var_decl_list->val.pair.head, scope);
				var_decl_list->type = ti;
				array_push(arr, ti);
				type_check_VAR_DECL_LIST(var_decl_list->val.pair.tail, scope, arr);
			}
		}
		break;

		case var_decl_var_typeK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = type_check_VAR_TYPE(var_decl_list->val.var_type, scope);
				var_decl_list->type = ti;
				array_push(arr, ti);
			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* ti = type_check_VAR_TYPE(var_decl_list->val.var_type, scope);
				var_decl_list->type = ti;
				array_push(arr, ti);
			}
		}
		break;

		default:  break;
	}
	return var_decl_list->type;
}

type_info* type_check_VAR_TYPE(VAR_TYPE * var_type, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <var_type> %d -- %d\n", 0, phase);

	type_info* ti = type_check_TYPE(var_type->type, scope);
	if (phase == type_phase_collectK)
	{
		//we want to create a copy of the type. and then assign our own data to it.
		type_info* var_type_ti = type_check_create_copy_of_type_info(ti);
		var_type_ti->type_id = var_type->id;
		var_type_ti->sym_table = scope;
		var_type_ti->is_a_variable = 1;
		var_type_ti->is_a_function = 0;
		var_type_ti->is_a_type = 0;
		var_type_ti->is_verified = 0;
		var_type_ti->type_info_child = ti; //our child is <type>
		var_type->ti = var_type_ti;
		var_type->ti->symbol = putSymbol(scope, var_type->id, var_type->ti);
		//dumpSymbolTable(scope);
		//type_checker_print_debug_ti(ti, "ti", var_type->lineno);
		//type_checker_print_debug_ti(var_type_ti, "var_type_ti", var_type->lineno);
	}

	if (phase == type_phase_verifyK || phase == type_phase_calculateK)
	{
		//TODO: recheck and rewrite comments here.
		//GHJ: first we look up our own id in the symbol table.
		//then we look up our child and update our own type to reflect that.
		//log_info("here be dragons");
		type_info* type_info_self = type_checker_lookup_once(var_type->id, scope, var_type->lineno);
		if (type_info_self)
		{
			if (!type_info_self->is_verified)
			{
				if (type_info_self->type_info_child)
				{
					//type_info* type_info_child_recursive = type_checker_lookup_once(var_type->ti->type_info_child->type_id, scope, var_type->lineno);
					type_info* type_info_child_recursive = type_check_recursively_lookup_id(var_type->ti->type_info_child->type_id, scope, var_type->lineno);
					if (type_info_child_recursive)
					{
						//update the symbol table to relfect the change.
						var_type->ti = type_info_child_recursive;
						var_type->ti->is_verified = 1;
						var_type->ti->is_a_variable = 1;
						var_type->ti->is_a_function = 0;
						var_type->ti->is_a_type = 0;
						var_type->ti->symbol = putSymbol(scope, var_type->id, var_type->ti);
						//dumpSymbolTable(scope);
					}
					else
					{
						var_type->ti = type_info_self->type_info_child;
						var_type->ti->is_verified = 1;
						var_type->ti->is_a_variable = 1;
						var_type->ti->is_a_function = 0;
						var_type->ti->is_a_type = 0;
						var_type->ti->symbol = putSymbol(scope, var_type->id, var_type->ti);
						//type_checker_print_debug_ti(type_info_self->type_info_child, "type_info_self->type_info_child", 0);
					}
				}
				else
				{
					//type_checker_print_debug_ti(type_info_self, "type_info_self", var_type->lineno);
					var_type->ti = type_info_self;
					var_type->ti->is_verified = 1;
					var_type->ti->is_a_variable = 1;
					var_type->ti->is_a_function = 0;
					var_type->ti->is_a_type = 0;
					var_type->ti->symbol = putSymbol(scope, var_type->id, var_type->ti);
				}
			}
		}
		else log_internal_error(__FILE__, __func__, __LINE__, "error in <var_type> failed finding our self in the symbol table.");
	}
	return var_type->ti;
}


type_info* type_check_BODY(BODY * body, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <body> %d -- %d\n", 0, phase);

	type_check_DECL_LIST(body->decl_list, scope);
	type_info* ti = type_check_STM_LIST(body->stm_list, scope);

	if (ti)
	{
		body->ti = ti;
		return ti;
	}


	return NULL;
}

//can return null
type_info* type_check_DECL_LIST(DECL_LIST * decl_list, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <decl_list> %d -- %d\n", decl_list->kind, phase);
	switch (decl_list->kind)
	{
		case decl_list_pairK:
		{
			type_check_DECL(decl_list->pair.head, scope);
			type_check_DECL_LIST(decl_list->pair.tail, scope);
		}

		break;

		case decl_list_emptyK:
		{
		}
		break;
		default:  break;
	}
	return NULL;
}

type_info* type_check_DECL(DECL * decl, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <decl> %d -- %d\n", decl->kind, phase);
	switch (decl->kind)
	{
		case decl_assignK:
		{
			type_info* ti = type_check_TYPE(decl->val.type, scope);

			if (phase == type_phase_collectK)
			{
				//type_checker_print_debug_ti(ti, "decl_decl_decl_ti", decl->lineno);
				//we want to create a copy of the type. and then assign our own data to it.
				type_info* decl_type = type_check_create_copy_of_type_info(ti);
				decl_type->kind = t_typeK;
				decl_type->type_id = decl->decl_id;
				decl_type->sym_table = scope;
				decl_type->is_a_variable = 0;
				decl_type->is_a_function = 0;
				decl_type->is_a_type = 1;
				decl_type->is_verified = 0;
				decl_type->type_info_child = ti;
				decl->type = decl_type;
				decl->type->symbol = putSymbol(scope, decl->decl_id, decl->type);
				//dumpSymbolTable(scope);

			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* type_info_self = type_checker_lookup_once(decl->decl_id, scope, decl->lineno);
				if (type_info_self)
				{
					if (type_info_self->type_info_child)
					{
						//dumpSymbolTable(scope);
						type_info* type_info_child_recursive = type_check_recursively_lookup_id(type_info_self->type_info_child->type_id, scope, decl->lineno);
						if (type_info_child_recursive)
						{
							if (!type_info_self->is_verified)
							{
								//update the symbol table to relfect the change.
								decl->type = type_info_child_recursive;
								decl->type->is_verified = 1;
								decl->type->is_a_variable = 0;
								decl->type->is_a_function = 0;
								decl->type->is_a_type = 1;
								decl->type->symbol = putSymbol(scope, decl->decl_id, decl->type);
								//dumpSymbolTable(scope);
							}

						}
						{
							if (!type_info_self->is_verified)
							{
								decl->type = type_info_self->type_info_child;
								decl->type->is_verified = 1;
								decl->type->is_a_variable = 0;
								decl->type->is_a_function = 0;
								decl->type->is_a_type = 1;
								decl->type->symbol = putSymbol(scope, decl->decl_id, decl->type);
							}
						}
					}
					else
					{
						if (!type_info_self->is_verified)
						{
							decl->type = type_info_self;
							decl->type->is_verified = 1;
							decl->type->is_a_variable = 0;
							decl->type->is_a_function = 0;
							decl->type->is_a_type = 1;
							decl->type->symbol = putSymbol(scope, decl->decl_id, decl->type);
						}
					}
				}
				else log_internal_error(__FILE__, __func__, __LINE__, "error in <var_type> failed finding our self in the symbol table.");


			}

		}
		break;

		case decl_functionK:
		{

			type_info* ti = type_check_FUNCTION(decl->val.function, scope);
			decl->type = ti;

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* function_self_lookup = type_checker_lookup_once(decl->val.function->func_head->function_id, scope, decl->lineno);
				if (function_self_lookup)
				{
					decl->type = function_self_lookup;
				}
				else
				{
					type_checker_print_undefined_variable(decl->lineno, decl->val.function->func_head->function_id);
				}

			}

		}
		break;

		case decl_varsK:
		{

			if (phase == type_phase_collectK)
			{
				array* arr = array_new();
				type_info* ti = type_check_VAR_DECL_LIST(decl->val.var_decl_list, scope, arr);
				decl->type = ti;
				decl->type->is_a_type = 0;
				decl->type->is_a_function = 0;
				decl->type->is_a_variable = 1;
			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				array* arr = array_new();
				type_info* ti = type_check_VAR_DECL_LIST(decl->val.var_decl_list, scope, arr);
				if (ti)
				{
					//no-op. to prevent gcc compilation warnings.
				}
			}
		}
		break;

		default:  break;
	}
	return decl->type;
}

type_info* type_check_STM_LIST(STM_LIST * stm_list, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <stm_list> %d -- %d\n", stm_list->kind, phase);

	switch (stm_list->kind)
	{
		case stm_list_pairK:
		{
			type_info* ti = type_check_STM(stm_list->val.pair.head, scope); //first element
			type_info* ti_stm_list = type_check_STM_LIST(stm_list->val.pair.tail, scope); //next/last element

			if (ti_stm_list != NULL) //return the last element first.
			{
				return ti_stm_list;
			}
			else if (ti != NULL)
			{
				return ti;
			}


		}
		break;

		case stm_list_statementK:
		{
			type_info* ti = type_check_STM(stm_list->val.stm, scope);

			if (ti != NULL)
			{
				return ti;
			}

		}
		break;

		default:  break;
	}
	return NULL;
}


type_info* type_check_STM(STM * stm, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <stm> %d -- %d\n", stm->kind, phase);
	switch (stm->kind)
	{
		case stm_returnK: //only the return statement should change the type of stm and return a valid type.
		{
			type_info* ti = type_check_EXP(stm->val.exp, scope);
			stm->type = ti;
			stm->type->has_return++;
			stm->type->return_type = ti;

			return stm->type;
		}
		break;

		case stm_writeK:
		{
			type_info* ti = type_check_EXP(stm->val.exp, scope);
			stm->type = ti;
			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				if (ti)
				{
					switch (ti->kind)
					{
						case t_idk:break;
						case t_intK:break;
						case t_boolK:break;
						case t_stringK:break;
						case t_nullK:break;
						case t_typeK:break;
						case t_arrayK:
						{
							type_checker_print_stm(stm->lineno, ti, "write");
						}
						break;
						case t_recordK: //don't allow
						{
							type_checker_print_stm(stm->lineno, ti, "write");
						}
						break;
					}
				}
			}
			return stm->type;
		}
		break;

		case stm_allocateK:
		{
			type_info* ti = type_check_VARIABLE(stm->val.var, scope);
			stm->type = ti; //TODO: FIX ME

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				if (ti)
				{
					switch (ti->kind) //TODO: refactor this.
					{
						case t_idk:	break;
						case t_intK:break;
						case t_boolK:break;
						case t_stringK:	break;
						case t_nullK:
						{
									//print warning with fancy colors.
							printf("\x1b[37;1m"); //start print color white.
							printf("%d: ", stm->lineno); //print lineno
							printf("\x1b[0m"); //reset color.
							printf("\x1b[36;1m"); //start print color magenta.
							printf("warning: "); //start print color magenta.
							printf("\x1b[0m"); //reset color.
							printf("Trying to allocate \x1b[37;1m'null'\x1b[0m\n");
						}
						break;
						case t_arrayK:
						break;
						case t_recordK:
						break;
						default:break;
					}
				}
			}
			return stm->type;
		}
		break;

		case stm_allocate_lengthK: //TODO: refactor
		{
			//requirement: exp must be of type int.
			type_info* ti_var = type_check_VARIABLE(stm->val.allocate_length.var, scope);
			type_info* ti_exp = type_check_EXP(stm->val.allocate_length.exp, scope);
			stm->type = ti_var;
			if (ti_var)
			{
				switch (ti_var->kind)
				{
					case t_idk:
					break;
					case t_intK:
					break;
					case t_boolK:
					break;
					case t_stringK:
					break;
					case t_nullK:
					{
							//print warning
						printf("\x1b[37;1m"); //start print color white.
						printf("%d: ", stm->lineno); //print lineno
						printf("\x1b[0m"); //reset color.
						printf("\x1b[36;1m"); //start print color magenta.
						printf("warning: "); //start print color magenta.
						printf("\x1b[0m"); //reset color.
						printf("Trying to allocate \x1b[37;1m'null'\x1b[0m\n");
					}
					break;
					case t_arrayK:
					break;
					case t_recordK:
					break;
					default:break;
				}
			}

			if (ti_exp)
			{
				switch (ti_exp->kind)
				{
					case t_idk:

					break;
					case t_intK:
					//this is allowed

					break;
					case t_boolK:
					type_checker_print_stm(stm->lineno, ti_exp, "allocate");
					break;
					case t_stringK:
					type_checker_print_stm(stm->lineno, ti_exp, "allocate");
					break;
					case t_nullK:
					type_checker_print_stm(stm->lineno, ti_exp, "allocate");
					break;
					case t_arrayK:
					type_checker_print_stm(stm->lineno, ti_exp, "allocate");
					break;
					case t_recordK: //don't allow
					type_checker_print_stm(stm->lineno, ti_exp, "allocate");
					break;
					default:break;
				}
			}

			return stm->type;
		}
		break;

		case stm_assignmentK:
		{
			type_info* ti_var = type_check_VARIABLE(stm->val.assignment.var, scope);
			type_info* ti_exp = type_check_EXP(stm->val.assignment.exp, scope);
			//log_warning("MMMMMMMMMMMM");
			//type_checker_print_debug_ti(ti_var, "ti_var", stm->lineno);
			//type_checker_print_debug_ti(ti_exp, "ti_exp", stm->lineno);
			if (phase == type_phase_collectK)
			{
				stm->type = ti_var;
				return stm->type;
			}
			if (ti_var && ti_exp)
			{

			}
			else
			{
				log_internal_error(__FILE__, __func__, __LINE__, "types were null");
			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				//log_warning("car 2");
				//type_info * var_recursive_lookup = type_check_recursively_lookup_type_info(ti_var, scope, stm->lineno, __LINE__);
				//type_info * exp_recursive_lookup = type_check_recursively_lookup_type_info(ti_exp, scope, stm->lineno, __LINE__);
				type_info * var_recursive_lookup = ti_var;
				type_info * exp_recursive_lookup = ti_exp;
				//type_checker_print_debug_ti(var_recursive_lookup, "var_recursive_lookup", stm->lineno);
				//type_checker_print_debug_ti(exp_recursive_lookup, "exp_recursive_lookup", stm->lineno);
				if (var_recursive_lookup->is_a_function)
				{
					//GHJ: possibly in the future it could be useful to have this functionality,
					//but for now it is not supported.
					if (phase == type_phase_verifyK) //only print once.
						type_checker_print_stm_assign_function_not_supported(stm->lineno);
				}
				else if (var_recursive_lookup->is_a_type)
				{
					if (phase == type_phase_verifyK) //only print once.
						type_checker_print_stm_assign_to_type(stm->lineno, ti_exp, ti_var->type_id);
				}
				else if (var_recursive_lookup->is_a_variable)
				{

				}


				if (var_recursive_lookup->kind == exp_recursive_lookup->kind)
				{

					if (var_recursive_lookup->kind == t_recordK && exp_recursive_lookup->kind == t_recordK)
					{
						size_t var_record_length = var_recursive_lookup->record_var_decl_list_types->length;
						size_t exp_record_length = exp_recursive_lookup->record_var_decl_list_types->length;
						//GHJ: look up each type and decide. check if the lengths match.
						if (var_record_length == exp_record_length)
						{
							char found_error = 0;
							//GHJ: check each type, only accept if each type is equal.
							for (size_t i = 0; i < var_record_length; ++i)
							{
								type_info* var_record_element = array_at(var_recursive_lookup->record_var_decl_list_types, i);
								type_info* exp_record_element = array_at(exp_recursive_lookup->record_var_decl_list_types, i);

								type_info* var_record_element_lookup = type_check_recursively_lookup_type_info(
									var_record_element, var_recursive_lookup->record_scope, stm->lineno);

								type_info* exp_record_element_lookup = type_check_recursively_lookup_type_info(
									exp_record_element, exp_recursive_lookup->record_scope, stm->lineno);

								//type_checker_print_debug_ti(var_record_element_lookup, "var_record_element_lookup", stm->lineno);
								//type_checker_print_debug_ti(exp_record_element_lookup, "exp_record_element_lookup", stm->lineno);
								if (var_record_element_lookup)
									if (exp_record_element_lookup)
										if (var_record_element_lookup->kind == exp_record_element_lookup->kind)
										{
										}
										else found_error = 1;
									else found_error = 1;
								else found_error = 1;

								if (found_error)
								{
									if (phase == type_phase_verifyK) //only print once.
										type_checker_print_stm_assign_record_mismatch_elements(
											stm->lineno,
											var_record_element_lookup,
											exp_record_element_lookup,
											i);
								}
							}

							if (found_error == 0)
							{
								type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
								result->is_a_type = 0;
								result->is_a_function = 0;
								result->is_a_variable = 1;
								result->type_id = ti_var->type_id;
								result->sym_table = ti_var->sym_table;
								stm->type = result;
								return stm->type;
							}

						}
						else
						{
							//GHJ: reject if the two lengths do not equate.
							type_checker_print_stm_assign_record_mismatch_count(
								stm->lineno,
								var_recursive_lookup->record_var_decl_list_types,
								exp_recursive_lookup->record_var_decl_list_types);
						}

					}
					else if (var_recursive_lookup->kind == t_arrayK && exp_recursive_lookup->kind == t_arrayK) //if assigning two arrays make sure their type is also the same.
					{
						if (var_recursive_lookup->array_type_info->kind != exp_recursive_lookup->array_type_info->kind)
						{
							//cannot assign an array to an array of different type.
							type_checker_print_stm_assign_array_mismatch_types(stm->lineno,
								var_recursive_lookup->array_type_info, exp_recursive_lookup->array_type_info);
						}
					}

					stm->type = exp_recursive_lookup;
					return stm->type;
				}
				else
				{
					//type_checker_print_debug_ti(var_recursive_lookup, "var_recursive_lookup", stm->lineno);
					//type_checker_print_debug_ti(exp_recursive_lookup, "exp_recursive_lookup", stm->lineno);
					if (var_recursive_lookup->kind == t_boolK)
					{
						if (exp_recursive_lookup->kind == t_intK)
						{//it is allowed to assign booleans to integer values.
							type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
							result->is_a_type = 0;
							result->is_a_function = 0;
							result->is_a_variable = 1;
							result->type_id = ti_var->type_id;
							result->sym_table = ti_var->sym_table;
							stm->type = result;
							stm->type->symbol = putSymbol(ti_var->sym_table, ti_var->type_id, stm->type);
							return stm->type;
						}
						else type_checker_print_stm_assign(stm->lineno, var_recursive_lookup, exp_recursive_lookup);
					}
					else if (var_recursive_lookup->kind == t_intK)
					{
						if (exp_recursive_lookup->kind == t_boolK)
						{
							type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
							result->is_a_type = 0;
							result->is_a_function = 0;
							result->is_a_variable = 1;
							result->type_id = ti_var->type_id;
							result->sym_table = ti_var->sym_table;
							stm->type = result;
							stm->type->symbol = putSymbol(ti_var->sym_table, ti_var->type_id, stm->type);
							return stm->type;
						}
						else type_checker_print_stm_assign(stm->lineno, var_recursive_lookup, exp_recursive_lookup);
					}
					else if (var_recursive_lookup->kind == t_stringK)
					{
						if (exp_recursive_lookup->kind != t_stringK)
							type_checker_print_stm_assign(stm->lineno, var_recursive_lookup, exp_recursive_lookup);
					}
					else if (var_recursive_lookup->kind == t_recordK)
					{
						if (exp_recursive_lookup->kind == t_nullK)
						{
							type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
							result->is_a_type = 0;
							result->is_a_function = 0;
							result->is_a_variable = 1;
							result->type_id = ti_var->type_id;
							result->sym_table = ti_var->sym_table;
							stm->type = result;
							stm->type->symbol = putSymbol(ti_var->record_scope, ti_var->type_id, stm->type);
							return stm->type;
						}
						else type_checker_print_stm_assign(stm->lineno, var_recursive_lookup, exp_recursive_lookup);
					}
					else if (var_recursive_lookup->kind == t_arrayK)
					{
						if (exp_recursive_lookup->kind == t_nullK)
						{
							type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
							result->is_a_type = 0;
							result->is_a_function = 0;
							result->is_a_variable = 1;
							result->type_id = ti_var->type_id;
							result->sym_table = ti_var->sym_table;
							stm->type = result;
							stm->type->symbol = putSymbol(ti_var->sym_table, ti_var->type_id, stm->type);
							return stm->type;
						}
						else type_checker_print_stm_assign(stm->lineno, var_recursive_lookup, exp_recursive_lookup);
					}
					else if (var_recursive_lookup->kind == t_nullK)
					{
						if (exp_recursive_lookup->kind == t_arrayK)
						{
							type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
							result->is_a_type = 0;
							result->is_a_function = 0;
							result->is_a_variable = 1;
							result->type_id = ti_var->type_id;
							result->sym_table = ti_var->sym_table;
							stm->type = result;
							stm->type->symbol = putSymbol(ti_var->sym_table, ti_var->type_id, stm->type);
							return stm->type;
						}
						else if (exp_recursive_lookup->kind == t_recordK)
						{
							type_info* result = type_check_create_copy_of_type_info(exp_recursive_lookup);
							result->is_a_type = 0;
							result->is_a_function = 0;
							result->is_a_variable = 1;
							result->type_id = ti_var->type_id;
							result->sym_table = ti_var->sym_table;
							stm->type = result;
							stm->type->symbol = putSymbol(ti_var->record_scope, ti_var->type_id, stm->type);
							return stm->type;
						}
						else
							type_checker_print_stm_assign(stm->lineno, var_recursive_lookup, exp_recursive_lookup);
					}
				} // else of - if (var_recursive_lookup->kind == exp_recursive_lookup->kind)
			} //if (phase == type_phase_calculateK || phase == type_phase_verifyK)
		} //end stm_assignmentK
		break;

		case stm_ifK:
		{
			//requirement: exp must be of type bool.
			type_info* ti_exp = type_check_EXP(stm->val.if_stm.exp, scope);
			//stm->type = ti_exp;
			array* arr = array_new();
			type_info* ti_stm = type_check_STM(stm->val.if_stm.stm, scope);
			array_push(arr, ti_stm);
			stm->type = ti_stm;
			stm->type->stm_list_types = arr;
			stm->type->return_type = ti_stm->return_type;

			if (phase == type_phase_verifyK)
			{
				if (ti_exp)
				{
					if (ti_exp->kind == t_idk) {}
					else if (ti_exp->kind == t_intK) {}
					else if (ti_exp->kind == t_boolK) {}
					else
					{
						type_checker_print_stm(stm->lineno, ti_exp, "If");
					}
				}

			}
			return stm->type;
		}//end stm_ifK
		break;

		case stm_if_elseK:
		{
			//requirement: exp must be of type bool.
			type_info* ti_exp = type_check_EXP(stm->val.if_else_stm.exp, scope);
				//stm->type = ti_exp;
			array* arr = array_new();
			type_info* ti_stm1 = type_check_STM(stm->val.if_else_stm.stm_then, scope);
			type_info* ti_stm2 = type_check_STM(stm->val.if_else_stm.stm_else, scope);
			array_push(arr, ti_stm1);
			array_push(arr, ti_stm2);
			stm->type = ti_stm2;
			stm->type->stm_list_types = arr;

			if (ti_stm2->return_type == NULL)
			{
				if (ti_stm1->return_type != NULL)
					stm->type->return_type = ti_stm1->return_type;
			}
			else
			{
				if (ti_stm1->return_type == NULL)
					stm->type->return_type = ti_stm2->return_type;
				else
					stm->type->return_type = ti_stm1->return_type;
			}

			if (phase == type_phase_verifyK)
			{
				if (ti_exp)
				{
					if (ti_exp->kind == t_idk) {}
					else if (ti_exp->kind == t_intK) {}
					else if (ti_exp->kind == t_boolK) {}
					else
					{
						type_checker_print_stm(stm->lineno, ti_exp, "If-Else-Statement");
					}
				}
			}
			return stm->type;
		}
		break;

		case stm_whileK:
		{
			type_info* ti_exp = type_check_EXP(stm->val.while_stm.exp, scope);

			array* arr = array_new();
			type_info* ti_stm = type_check_STM(stm->val.while_stm.stm, scope);
			array_push(arr, ti_stm);
			stm->type = ti_stm;
			stm->type->stm_list_types = arr;
			stm->type->return_type = ti_stm->return_type;

			if (phase == type_phase_verifyK)
			{
				if (ti_exp)
				{
					if (ti_exp->kind == t_idk) {}
					else if (ti_exp->kind == t_intK) {}
					else if (ti_exp->kind == t_boolK) {}
					else
					{
						type_checker_print_stm(stm->lineno, ti_exp, "While");
					}
				}
			}
			return stm->type;
		}
		break;

		case stm_listK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = type_check_STM_LIST(stm->val.list, scope);
				if (ti != NULL)
				{
					stm->type = ti;
					return ti;
				}
			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* ti = type_check_STM_LIST(stm->val.list, scope);
				if (ti != NULL)
				{
					stm->type = ti;
					return ti;
				}
			}

		}
		break;

		default:  break;
	}
	return NULL;
}





type_info* type_check_VARIABLE(VARIABLE * var, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <variable> %d -- %d\n", var->kind, phase);
	switch (var->kind)
	{
		case variable_idK:
		{
			if (FALCON_DEBUG_TYPECHECK)
				printf("___ID___ %d\n", phase);

			if (phase == type_phase_collectK) //TODO: REMOVE THIS FIX IT.
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_idk;
				ti->type_id = var->val.id;
				ti->sym_table = scope;
				ti->is_a_variable = 1;
				ti->is_a_function = 0;
				ti->is_a_type = 0;
				var->type = ti;
			}


			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* ti_lookup_once = type_checker_lookup_once(var->val.id, scope, var->lineno);

				if (!var->type->is_verified)
				{
					if (ti_lookup_once)
					{
						type_info* result = type_check_create_copy_of_type_info(ti_lookup_once);
						result->type_id = var->val.id;
						result->is_verified = 1;
						result->sym_table = scope;
						var->type = result;
						//var->type->symbol = putSymbol(scope, var->val.id, var->type);
					}
					else log_internal_error(__FILE__, __func__, __LINE__, "ti_recursive_lookup was null.");
				}
			}
		}
		break;

		case variable_arrayAccessK:
		{
			type_info* ti_var = type_check_VARIABLE(var->val.array_access.var, scope);
			type_info* ti_exp = type_check_EXP(var->val.array_access.exp, scope);

			if (phase == type_phase_collectK)
			{
				var->type = ti_var;
			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				//log_warning("var.access");
				//type_checker_print_debug_ti(ti_var, "var.array.access->ti_var", var->lineno);
				//type_checker_print_debug_ti(ti_exp, "var.array.access->ti_exp", var->lineno);
				//type_checker_print_debug_ti(ti_var->array_type_info, "var.array.access->array_Type_info", var->lineno);
				//log_info("scope");
				//dumpSymbolTable(scope);
				//log_info("sym_table");
				//dumpSymbolTable(ti_var->sym_table);
				//log_info("recorD_scope");
				//dumpSymbolTable(ti_var->record_scope);
				type_info* var_recursive = type_check_recursively_lookup_type_info(ti_var, ti_var->sym_table, var->lineno);

				//type_checker_print_debug_ti(var_recursive, "var_recusive", var->lineno);

				type_info* exp_recursive = type_check_recursively_lookup_type_info(ti_exp, scope, var->lineno);
				//type_checker_print_debug_ti(exp_recursive, "exp_recusive", var->lineno);

				if (exp_recursive->kind == t_intK)
				{

				}
				else
				{
					//GHJ: maybe in the future one could add strings as a form of indexing like: somearray["hello"],
					//by first checking the type of the array and then deciding this.
					//It could also be beneficial to have both at the same time!
					//Implementing object orientation would certainly make the implementation of such much easier.
					if (phase == type_phase_verifyK) //only print once.
						type_checker_print_variable_array_access_illegal_indexing_method(var->lineno, exp_recursive);
				}

				if (var_recursive->kind == t_arrayK)
				{
					//GHJ: it is important here not to use our own scope, but the scope of the array type itself.
					type_info* array_type_recursive_lookup = type_check_lookup_once_type_info(var_recursive->array_type_info, var_recursive->sym_table, var->lineno);
					if (array_type_recursive_lookup)
					{
						//type_checker_print_debug_ti(array_type_recursive_lookup, "array_type_info", var->lineno);
						var->type = array_type_recursive_lookup;
					}
				}
				else if (var_recursive->kind == t_nullK)
				{
					//GHJ: trying to access null. this should be allowed, however, will only get null in return.
				}
				else
				{
					//GHJ: print error: only arrays allowed to be accessed here.
					if (phase == type_phase_verifyK) //only print once.
						type_checker_print_variable_array_access_non_array_type(var->lineno, var_recursive);
				}
			}
		}

		break;

		case variable_access_idK:
		{
			//printf("---- var access kind is: %d \n ", var->val.access.var->kind);
			//go inside variable the first variable -> get its type
			type_info* ti_var = type_check_VARIABLE(var->val.access.var, scope);


			if (phase == type_phase_collectK)
			{
				var->type = ti_var;
			}

			if (phase == type_phase_calculateK || phase == type_phase_verifyK) //TODO: FIX AND INTEGRATE ERROR MESSAGES.
			{

				//log_warning("var.access.id");
				//type_checker_print_debug_ti(ti_var, "var.access.id->ti_var", var->lineno);
				//log_info("scope");
				//dumpSymbolTable(scope);
				//log_info("sym_table");
				//dumpSymbolTable(ti_var->sym_table);
				//log_info("record_scope");
				//dumpSymbolTable(ti_var->record_scope);
				type_info* var_recursive = type_check_recursively_lookup_type_info(ti_var, ti_var->sym_table, var->lineno);
				//type_checker_print_debug_ti(var_recursive, "var_recusive", var->lineno);
				//type_checker_print_debug_ti(ti_var, "ti_var", var->lineno);

				if (var_recursive->kind == t_recordK)
				{
					//GHJ: if it's a valid record, then go inside and lookup the child id in the record scope.
					type_info* child_recursive_id = type_check_recursively_lookup_id(var->val.access.id, var_recursive->record_scope, var->lineno);
					if (child_recursive_id)
					{
						type_info* child_recursive_type = type_check_recursively_lookup_type_info(child_recursive_id, var_recursive->record_scope, var->lineno);
						type_info* result = type_check_create_copy_of_type_info(child_recursive_type);
						result->is_a_type = 0;
						result->is_a_function = 0;
						result->is_a_variable = 0;
						result->is_record_element = 1;
						result->is_verified = 1;
						result->type_id = var->val.access.id;
						result->record_scope = var_recursive->record_scope;
						result->sym_table = var_recursive->record_scope;
						//type_checker_print_debug_ti(child_recursive_type, "child_recursive_type", var->lineno);
						//var->type = child_recursive_type;
						var->type = result;
						//log_warning("==================================================================================");
						//printf("var %s:%d\n", var->val.access.id, child_recursive_type->kind);
						//log_warning("==================================================================================");
					}
					else
					{
						log_internal_error(__FILE__, __func__, __LINE__, "failure id couldn't be found in the record.");
					}
				}
				else
				{
					//GHJ: this is intended to only be allowed for records, anything that is not a record is not allowed to be accessed.
					//if (phase == type_phase_verifyK) //only print once.
					//	type_checker_print_variable_access_illegal_type(var->lineno, var_recursive);
				}
			}
		}

		break;

		default: break;
	}
	return var->type;
}

type_info* type_check_EXP(EXP * exp, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <exp> %d -- %d\n", exp->kind, phase);
	switch (exp->kind)
	{
		case exp_timesK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{

					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						//a * b -> a = id, -> id * id = legal -> id * 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Multiplication is illegal for");
					}
					else if (a->kind == t_intK) //reverse order applies.
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Multiplication is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Multiplication is illegal for");


				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_intK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_divK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{

					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						//a / b -> a = id, -> id / id = legal -> id / 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Division is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Division is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Division is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_intK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_plusK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);


					if (a->kind == t_idk)
					{
						//a + b -> a = id, -> id + id = legal -> id + 5 = legal -> id + string = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_stringK) {}
						else type_checker_print_template(exp->lineno, a, b, "Addition is illegal for");
					}
					else if (a->kind == t_intK)
					{
						//a+b -> a = int, 5 + id = legal -> 5 + 5 = -> 5 + string = illegal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Addition is illegal for");
					}
					else if (a->kind == t_stringK)
					{
						//a + b -> a = string, string + id = legal -> string + string = legal.
						if (b->kind == t_idk) {}
						else if (b->kind == t_stringK) {}
						else type_checker_print_template(exp->lineno, a, b, "Addition is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Addition is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_intK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_minusK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{

				if (phase == type_phase_calculateK)
				{

					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						//a - b -> a = id, -> id - id = legal -> id - 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Subtraction is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Subtraction is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Subtraction is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_intK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_equalityK:
		{
			//the result of equality is of type int
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{

				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_stringK) {}
						else if (b->kind == t_recordK) {}
						else if (b->kind == t_arrayK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else if (a->kind == t_stringK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_stringK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else if (a->kind == t_recordK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_recordK) {}
						else if (b->kind == t_nullK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else if (a->kind == t_nullK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_recordK) {}
						else if (b->kind == t_arrayK) {}
						else if (b->kind == t_nullK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else if (a->kind == t_arrayK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_arrayK) {}
						else if (b->kind == t_nullK) {}
						else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Equality is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_boolK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_non_equalityK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);


					if (a->kind == t_idk)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_stringK) {}
						else if (b->kind == t_recordK) {}
						else if (b->kind == t_arrayK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else if (a->kind == t_stringK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_stringK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else if (a->kind == t_recordK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_recordK) {}
						else if (b->kind == t_nullK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else if (a->kind == t_nullK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_recordK) {}
						else if (b->kind == t_arrayK) {}
						else if (b->kind == t_nullK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else if (a->kind == t_arrayK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_arrayK) {}
						else if (b->kind == t_nullK) {}
						else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Non-Equality is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_boolK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_greaterK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			exp->type = b;
			if (a && b)
			{

				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						//a - b -> a = id, -> id - id = legal -> id - 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Greater operation is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Greater operation is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Greater operation is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Greater operation is illegal for");

				}


				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_boolK;
				ti->sym_table = scope;
				exp->type = ti;
			}
		}
		break;

		case exp_lesserK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						//a - b -> a = id, -> id - id = legal -> id - 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Lesser operation is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Lesser operation is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Lesser operation is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Lesser operation is illegal for");

				}

				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_boolK;
				ti->sym_table = scope;
				exp->type = ti;
			}
		}
		break;

		case exp_greater_equalK:
		{
			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk) //TODO: check if this is really triggered, since variable is recursive now.
					{
						//a - b -> a = id, -> id - id = legal -> id - 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Greater-equal than operation is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Greater-equal than operation is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Greater-equal than operation is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Greater-equal than operation is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_boolK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_lesser_equalK:
		{

			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						//a - b -> a = id, -> id - id = legal -> id - 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Lesser-equal than operation is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Lesser-equal than operation is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Lesser-equal than operation is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Lesser-equal than operation is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_boolK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_andK:
		{

			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			if (a && b)
			{
				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);


					if (a->kind == t_idk)
					{
						//a - b -> a = id, -> id - id = legal -> id - 5 = legal
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Operational AND is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Operational AND is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Operational AND is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Operational AND is illegal for");

				}
			}

			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_boolK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_orK:
		{

			type_info* a = type_check_EXP(exp->val.op.left, scope);
			type_info* b = type_check_EXP(exp->val.op.right, scope);
			if (a && b)
			{

				if (phase == type_phase_calculateK)
				{
					//type_checker_print_debug_ti(a, "exp a", exp->lineno);
					//type_checker_print_debug_ti(b, "exp b", exp->lineno);

					if (a->kind == t_idk)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_intK) {}
						else if (b->kind == t_boolK) {}
						else type_checker_print_template(exp->lineno, a, b, "Operational OR is illegal for");
					}
					else if (a->kind == t_intK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Operational OR is illegal for");
					}
					else if (a->kind == t_boolK)
					{
						if (b->kind == t_idk) {}
						else if (b->kind == t_boolK) {}
						else if (b->kind == t_intK) {}
						else type_checker_print_template(exp->lineno, a, b, "Operational OR is illegal for");
					}
					else type_checker_print_template(exp->lineno, a, b, "Operational OR is illegal for");

				}
			}
			type_info* ti = Calloc(1, sizeof(type_info));
			ti->kind = t_boolK;
			ti->sym_table = scope;
			exp->type = ti;
		}
		break;

		case exp_termK:
		{
			type_info* ti = type_check_TERM(exp->val.term, scope);
			exp->type = ti;
		}
		break;

		default:  break;
	}
	return exp->type;
}

type_info* type_check_TERM(TERM * term, SymbolTable * scope)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <term> %d -- %d\n", term->kind, phase);
	switch (term->kind)
	{
		case term_varK:
		{
			type_info* ti = type_check_VARIABLE(term->val.var, scope);
			term->type = ti;
			return term->type;
		}
		break;

		case term_id_act_listK:
		{
			if (phase == type_phase_collectK) //TODO: GHJ: i don't think we can do this in the collection phase.
			{
				type_info* act_list = type_check_ACT_LIST(term->val.id_act_list.act_list, scope, NULL);
				if (act_list)
				{
					//no-op. here to prevent gcc compilation warnings, might be useful in the future to check this.
				}

				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_idk;
				ti->sym_table = scope;
				ti->func_id = term->val.id_act_list.id;
				term->type = ti;
			}


			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				//log_warning("id_act_list\n");
				//GHJ: first we look the function and then we handle the parameters.
				type_info* function = type_checker_lookup_once(term->val.id_act_list.id, scope, term->lineno);
				if (function)
				{
					if (function->is_a_function)
					{
						//type_checker_print_debug_ti(function, "id_recursive_lookup", term->lineno);
						//assign the type
						term->type = function;

						//check the parameters:
						GLOBAL_ACT_LIST_SIZE++;
						array* arr = array_new();
						type_info* act_list = type_check_ACT_LIST(term->val.id_act_list.act_list, scope, arr);
						size_t number_of_call_arguments = arr->length;
						if (function->function_var_decl_list_types)
						{
							size_t number_of_function_parameters = function->function_var_decl_list_types->length;
							if (number_of_call_arguments != number_of_function_parameters) //either the number is high or low.
							{
								if (phase == type_phase_verifyK)
								{
									type_checker_print_function_call_invalid_parameter_count(term->lineno,
										number_of_function_parameters, number_of_call_arguments, function->func_id);
								}
							}
							else //else the parameter count is correct. then each parameter type is checked.
							{
								char found_error = 0;
								for (size_t i = 0; i < number_of_function_parameters; ++i)
								{
									type_info* parameter = array_at(function->function_var_decl_list_types, i);
									//GHJ: just to make sure it's correct, let's look up the parameter using the function scope.
									type_info* parameter_lookup = type_check_recursively_lookup_type_info(parameter, function->sym_table, term->lineno);
									//type_checker_print_debug_ti(parameter_lookup, "parameter_lookup", term->lineno);

									type_info* argument = array_at(arr, i);
									//type_checker_print_debug_ti(parameter, "parameter", term->lineno);
									//type_checker_print_debug_ti(argument, "argument", term->lineno);
									if (parameter)
									{
										if (argument)
										{
											if (parameter_lookup->kind == argument->kind) continue;
											else
											{
												if (parameter_lookup->kind == t_arrayK)
												{
													if (argument->kind == t_nullK) continue;
													else found_error = 1;
												}
												else if (parameter->kind == t_recordK)
												{
													if (argument->kind == t_nullK) continue;
													else found_error = 1;
												}
												else if (argument->kind == t_arrayK)
												{
													if (parameter->kind == t_nullK) continue;
													else found_error = 1;
												}
												else if (argument->kind == t_recordK)
												{
													if (parameter->kind == t_nullK) continue;
													else found_error = 1;
												}
												else found_error = 1;
											}
										}
										else found_error = 1;
									}
									else found_error = 1;
									found_error = 1;
								}

								if (found_error != 0)
								{
									if (phase == type_phase_verifyK)
										type_checker_print_function_call_invalid_parameter_types(term->lineno, function, act_list);
								}
							}
						}
						else
						{
							if (number_of_call_arguments != 0)
							{
								//function has 0 parameters.
								if (phase == type_phase_verifyK)
								{
									type_checker_print_function_call_invalid_parameter_count(term->lineno,
								0, number_of_call_arguments, function->func_id);
								}
							}
						}


						if (act_list)
						{
							act_list->act_list_types = arr;
						}

					}
					else
					{
						//GHJ: only functions are allowed to be called here,
						//if we try to call a non-function that would be an illegal action that should be caught.
						//this can be moved to a weeding phase after the type-checker.
						if (phase == type_phase_verifyK) //only print once.
							type_checker_print_term_function_call_non_function(term->lineno, function);
					}
				}
				else
				{
					type_checker_print_undefined_variable(term->lineno, term->val.id_act_list.id);
				}
			}
		}
		break;

		case term_parenthesis_expressionK:
		{
			type_info* ti = type_check_EXP(term->val.exp, scope);
			ti->sym_table = scope;
			term->type = ti;
		}
		break;

		case term_negationK:
		{
			type_info* ti = type_check_TERM(term->val.term, scope);
			ti->sym_table = scope;
			term->type = ti;

			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				switch (ti->kind)
				{
					case t_idk:
					{
					}
					break;
					case t_intK:
					{
						type_checker_print_negation(term->lineno, ti);
					}
					break;
					case t_boolK:
					{
							//this is allowed.
					}
					break;
					case t_nullK:
					{
						type_checker_print_negation(term->lineno, ti);
					}
					break;
					case t_arrayK:
					{
						type_checker_print_negation(term->lineno, ti);
					}
					break;
					case t_recordK:
					{
						type_checker_print_negation(term->lineno, ti);
					}
					break;
					case t_stringK:
					{
						type_checker_print_negation(term->lineno, ti);
					}
					break;
					default:break;
				}
			}
		}
		break;

		case term_absoluteK:
		{
			type_info* ti = type_check_EXP(term->val.exp, scope);
			ti->sym_table = scope;
			term->type = ti;
			if (phase == type_phase_verifyK || phase == type_phase_calculateK)
			{
				switch (ti->kind)
				{
					case t_idk:
					{
						type_info* ti = Calloc(1, sizeof(type_info));
						ti->kind = t_intK;
						ti->sym_table = scope;
						term->type = ti;
					}
					break;
					case t_intK:
					{
						//this is allowed.
						type_info* ti = Calloc(1, sizeof(type_info));
						ti->kind = t_intK;
						ti->sym_table = scope;
						term->type = ti;
					}
					break;
					case t_boolK:
					{
							//this is not allowed.
						type_checker_print_abs(term->lineno, ti);
					}
					break;
					case t_nullK:
					{
						type_checker_print_abs(term->lineno, ti);
					}
					break;
					case t_arrayK:
					{
						type_info* ti = Calloc(1, sizeof(type_info));
						ti->kind = t_intK;
						ti->sym_table = scope;
						term->type = ti;
					}
					break;
					case t_recordK:
					{
						type_checker_print_abs(term->lineno, ti);
					}
					break;
					case t_stringK:
					{
						//this is allowed.
						type_info* ti = Calloc(1, sizeof(type_info));
						ti->kind = t_intK;
						ti->sym_table = scope;
						term->type = ti;
					}
					break;
					default:break;
				}
			}
		}
		break;

		case term_numK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_intK;
				ti->sym_table = scope;
				term->type = ti;
				return term->type;
			}
		}
		break;

		case term_trueK: //boolean
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_boolK;
				ti->sym_table = scope;
				term->type = ti;
				return term->type;
			}
		}
		break;

		case term_falseK: //boolean
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_boolK;
				ti->sym_table = scope;
				term->type = ti;
				return term->type;
			}
		}
		break;

		case term_nullK: //null type
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = Calloc(1, sizeof(type_info));
				ti->kind = t_nullK;
				ti->sym_table = scope;
				term->type = ti;
				return term->type;
			}
		}
		break;

		case term_stringK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = type_check_STRING(term->val.string, scope);
				ti->sym_table = scope;
				term->type = ti;

				return term->type;
			}
		}
		break;

		default:  break;
	}
	return term->type;
}


type_info* type_check_ACT_LIST(ACT_LIST * act_list, SymbolTable * scope, array * arr)
{
	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <act_list> %d -- %d\n", act_list->kind, phase);
	switch (act_list->kind)
	{
		case act_list_exp_listK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti = type_check_EXP_LIST(act_list->exp_list, scope, NULL);
				if (ti)
				{
					//no-op. preventing gcc compilation warnings.
				}
			}
			if (phase == type_phase_verifyK || phase == type_phase_calculateK)
			{
				array* arr_exp = array_new();
				type_info* ti = type_check_EXP_LIST(act_list->exp_list, scope, arr);
				act_list->type = ti;
				act_list->type->act_list_count = ti->act_list_count;

				if (arr_exp)
				{
					//no-op. preventing gcc compilation warnings.
				}
			}
		}
		break;

		case act_list_emptyK:
		break;

		default:  break;
	}
	return act_list->type;
}


type_info* type_check_EXP_LIST(EXP_LIST * exp_list, SymbolTable * scope, array * arr)
{

	if (FALCON_DEBUG_TYPECHECK)
		printf("|||||------> <exp_list> %d -- %d\n", exp_list->kind, phase);
	switch (exp_list->kind)
	{
		case exp_list_pairK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti_exp = type_check_EXP(exp_list->val.pair.head, scope);
				exp_list->type = ti_exp;
				type_check_EXP_LIST(exp_list->val.pair.tail, scope, NULL);
			}
			if (phase == type_phase_verifyK || phase == type_phase_calculateK)
			{
				type_info* ti_exp = type_check_EXP(exp_list->val.pair.head, scope);
				array_push(arr, ti_exp);
				type_info* ti_exp_list = type_check_EXP_LIST(exp_list->val.pair.tail, scope, arr);
				exp_list->type = ti_exp_list;
			}
		}
		break;
		case exp_list_expressionK:
		{
			if (phase == type_phase_collectK)
			{
				type_info* ti_exp = type_check_EXP(exp_list->val.exp, scope);
				exp_list->type = ti_exp;
			}
			if (phase == type_phase_calculateK || phase == type_phase_verifyK)
			{
				type_info* ti_exp = type_check_EXP(exp_list->val.exp, scope);
				exp_list->type = ti_exp;
				array_push(arr, ti_exp);
				exp_list->type->act_list_types = arr;
				exp_list->type->act_list_count += 1;
			}
		}
		break;
		default:  break;
	}
	return exp_list->type;
}
