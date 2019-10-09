#pragma once
#include "tree.h"
#include "linked_list.h"

//structure to convey data in the recurtion
typedef struct code_info
{

	/* To keep track of the different offsets in the posabily infinitely many scopes.
	 * Thus a stack is needed to maintain the offset from previout scope.
	 *
	 * There is also need to keep track of the offsets of the parameters in each function (param_offset)*/
	linked_list *offset_stack;
	int param_offset;

	//used to know whether we have a function parameter, local vairable, or record variable when setting its offset in stack
	enum
	{
		none,
		func_param,
		local_var,
		record_var,
	} kind;

	//used to know what type of data is moved around in the code_gen
	enum
	{
		none_type,
		int_type,
	} type;

	//used by static link to know what kind of statement the variabl resides in
	enum
	{
		none_stm,
		assignment_stm,
	} stm_kind;

	char variable_is_array_access;
	char variable_is_assign;

	//used for assignment statement to store the expression result at the position of static link + offset
	int static_link_count;
	int static_link_var_offset; //offset of a specific variable from the given static link

	/* The scope in which the statment lies in,
	 * and thus the variable has to be found from that scope */
	SymbolTable *scope;
	/* Scope of the implicit main function. (AKA root scope in symbol table) */
	SymbolTable *main_scope;

	/* If function has return statement
	 * this saves the specific functions lable*/
	char *func_lable; //not used

	int num;
	SymbolTable *current_scope;

	char **ptr_to_static_string_label;
} code_info;

/*
typedef struct cg_data
{
	enum
	{
		rax,
		rbx,
		rcx,
		rdx,
		rdi,
		rsi,
		r8,
		r9,
		r10,
		r11,
		r12,
		r13,
		r14,
		r15,
	} reg;
	enum
	{
		t_exp_bool_equalityK,
		t_exp_bool_non_equalityK,
		t_exp_bool_greaterK,
		t_exp_bool_lesserK,
		t_exp_bool_greater_equalK,
		t_exp_bool_lesser_equalK,
	} exp_bool_kind;
	char Is_Pushed;
	char** ptr_to_static_string_label;
} cg_data;
*/

//char* code_gen_resolve_register_to_string(cg_data* cg);
char *code_gen_create_unique_label();
char *code_gen_create_unique_label_with_seed(char *seed);

void code_gen(BODY *AST, linked_list *list, SymbolTable *root);

void code_gen_STRING(STRING *s, linked_list *list, linked_list *main, code_info *data);
void code_gen_FUNCTION(FUNCTION *f, linked_list *list, linked_list *main, code_info *data);
void code_gen_HEAD(HEAD *head, linked_list *list, linked_list *main, code_info *data);
void code_gen_TAIL(TAIL *tail, linked_list *list, linked_list *main, code_info *data);
void code_gen_TYPE(TYPE *type, linked_list *list, linked_list *main, code_info *data);
void code_gen_PAR_DECL_LIST(PAR_DECL_LIST *par_decl_list, linked_list *list, linked_list *main, code_info *data);
void code_gen_VAR_DECL_LIST(VAR_DECL_LIST *var_decl_list, linked_list *list, linked_list *main, code_info *data);
void code_gen_VAR_TYPE(VAR_TYPE *var_type, linked_list *list, linked_list *main, code_info *data);
void code_gen_BODY(BODY *body, linked_list *list, linked_list *main, code_info *data);
void code_gen_DECL_LIST(DECL_LIST *decl_list, linked_list *list, linked_list *main, code_info *data);
void code_gen_DECL(DECL *decl, linked_list *list, linked_list *main, code_info *data);
void code_gen_STM_LIST(STM_LIST *stm_list, linked_list *list, linked_list *main, code_info *data);
void code_gen_STM(STM *stm, linked_list *list, linked_list *main, code_info *data);
void code_gen_VARIABLE(VARIABLE *var, linked_list *list, linked_list *main, code_info *data);
void code_gen_EXP(EXP *exp, linked_list *list, linked_list *main, code_info *data);
void code_gen_TERM(TERM *term, linked_list *list, linked_list *main, code_info *data);
void code_gen_ACT_LIST(ACT_LIST *act_list, linked_list *list, linked_list *main, code_info *data);
void code_gen_EXP_LIST(EXP_LIST *exp_list, linked_list *list, linked_list *main, code_info *data);

/********************** STACK FRAME (ACTIVATION RECORD) ****************************
(When function is called)
|-------------------------------------------------------------------------------|
|	1) Push the values from the registers that is used by the callee, to stack.	|
|	-------------------------------------------------------------------------	|
|	2) Push the local variables of the function, to stack.						|
|	-------------------------------------------------------------------------   |
|	3) Push the values from the registers that is used by the caller, to stack.	|
|	-------------------------------------------------------------------------	|
|	4) Push the parameters/argument to the next function called, to stack.		|
|   -------------------------------------------------------------------------   |
|	5) Push return address for the called function.								|
|-------------------------------------------------------------------------------|
When the next function is called:
|-------------------------------------------------------------------------------|
|	1) Push the values from the registers that is used by the callee, to stack.	|
|	-------------------------------------------------------------------------	|
|	2) Push the local variables of the func.....								|
|		.								.							.			|
|		.								.							.			|
|		.								.							.			|
|	(And so on...)																|
|-------------------------------------------------------------------------------|
**********************************************************************************/