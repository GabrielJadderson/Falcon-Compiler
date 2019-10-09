#include "memory.h"
#include "tree.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"

extern char FALCON_DEBUG_PARSER;

extern int lineno;


/* =============== FUNCTION START =============== */
STRING* make_STRING(char* string)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <string> \"%s\" of length: %lu at line:%d\n", string, strlen(string), lineno);
	STRING * e = NEW(STRING);
	e->lineno = lineno;
	e->kind = stringK;
	e->str = string;
	e->length = strlen(string);
	return e;
}
STRING* make_STRING_concatenation(char* a, char* b)
{
	size_t length_a = strlen(a);
	size_t length_b = strlen(b);
	size_t total_length = length_a + length_b;
	char* ptr = malloc(total_length + 1); // +1 for the null character.
	if (!ptr) { printf("FAILED TO ALLOCATE SPACE FOR STRING CONCATENATION\n"); exit(1); }
	strcpy(ptr, a); //first we copy the contents of a into the new space
	strcat(ptr, b); //then we use strcat to append b after we've added a.

	if (FALCON_DEBUG_PARSER)
		printf("Made <string> concatenation of \"%s\" + \"%s\" of length: %lu, %lu at line:%d into -> ", a, b, length_a, length_b, lineno);
	//free both a and b, since we allocate them in flex during the scanning phase.
	free(a);
	free(b);

	STRING * e = NEW(STRING);
	e->lineno = lineno;
	e->str = ptr;
	e->length = total_length;
	e->kind = string_concatenationK;
	if (FALCON_DEBUG_PARSER)
		printf("%s\n", ptr);
	return e;
}
/* =============== FUNCTION END ================ */



/* =============== FUNCTION START =============== */
FUNCTION * make_FUNC(struct HEAD * head, struct BODY * body, struct TAIL * tail)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <function> at line:%d\n", lineno);
	FUNCTION * e = NEW(FUNCTION);
	e->lineno = lineno;
	e->func_head = head;
	e->func_body = body;
	e->func_tail = tail;
	return e;
}
/* =============== FUNCTION END ================ */


/* =============== HEAD START =============== */
HEAD* make_HEAD(char* func_id, struct PAR_DECL_LIST * params, struct TYPE * type)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <head> of id: %s -> at line:%d\n", func_id, lineno);
	HEAD * e = NEW(HEAD);
	e->lineno = lineno;
	e->function_id = func_id;
	e->params = params;
	e->return_type = type;
	return e;
}
/* =============== HEAD END ================ */


/* =============== TAIL START =============== */
TAIL* make_TAIL(char* end_id)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <tail> of id: %s -> at line:%d\n", end_id, lineno);
	TAIL * e = NEW(TAIL);
	e->lineno = lineno;
	e->end_id = end_id;
	return e;
}
/* =============== TAIL END ================ */


/* =============== TYPE START =============== */
TYPE* make_TYPE_id(char* type_id)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <type> of id: %s -> at line:%d\n", type_id, lineno);
	TYPE * e = NEW(TYPE);
	e->lineno = lineno;
	e->kind = type_idK;
	e->val.type_id = type_id;
	return e;
}
TYPE* make_TYPE_int()
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <type> of int at line: %d\n", lineno);
	TYPE * e = NEW(TYPE);
	e->lineno = lineno;
	e->kind = type_intK;
	return e;
}
TYPE* make_TYPE_bool()
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <type> of bool at line: %d\n", lineno);
	TYPE * e = NEW(TYPE);
	e->lineno = lineno;
	e->kind = type_boolK;
	return e;
}
TYPE* make_TYPE_string()
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <type> of string at line: %d\n", lineno);
	TYPE * e = NEW(TYPE);
	e->lineno = lineno;
	e->kind = type_stringK;
	return e;
}

TYPE* make_TYPE_array_of(struct TYPE * type)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <type> of array of <type->kind = %d> at line: %d\n", type->kind, lineno);
	TYPE * e = NEW(TYPE);
	e->lineno = lineno;
	e->kind = type_arrayK;
	e->val.type = type;
	return e;
}
TYPE* make_TYPE_record(struct VAR_DECL_LIST * var_decl_list)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <type> record of ... at line: %d\n", lineno);
	TYPE * e = NEW(TYPE);
	e->lineno = lineno;
	e->kind = type_recordK;
	e->val.var_decl_list = var_decl_list;
	return e;
}
/* =============== TYPE END ================ */


/* =============== PAR_DECL_LIST START =============== */
PAR_DECL_LIST* make_PAR_DECL_LIST_empty()
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <par_decl_list> empty at line: %d\n", lineno);
	PAR_DECL_LIST * e = NEW(PAR_DECL_LIST);
	e->lineno = lineno;
	e->kind = par_decl_list_emptyK;
	return e;
}
PAR_DECL_LIST* make_PAR_DECL_LIST(struct VAR_DECL_LIST * var_decl_list)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <par_decl_list> non-empty at line: %d\n", lineno);
	PAR_DECL_LIST * e = NEW(PAR_DECL_LIST);
	e->lineno = lineno;
	e->kind = par_decl_list_var_decl_listK;
	e->val.var_decl_list = var_decl_list;
	return e;
}
/* =============== PAR_DECL_LIST END ================ */


/* =============== VAR_DECL_LIST START =============== */
VAR_DECL_LIST* make_VAR_DECL_LIST_pair(struct VAR_TYPE * head, VAR_DECL_LIST * tail)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <var_decl_list> pair at line: %d\n", lineno);
	VAR_DECL_LIST * e = NEW(VAR_DECL_LIST);
	e->lineno = lineno;
	e->kind = var_decl_list_pairK;
	e->val.pair.head = head;
	e->val.pair.tail = tail;
	return e;
}
VAR_DECL_LIST* make_VAR_DECL_LIST_last(struct VAR_TYPE * var_type)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <var_decl_list> var at line: %d\n", lineno);
	VAR_DECL_LIST * e = NEW(VAR_DECL_LIST);
	e->lineno = lineno;
	e->kind = var_decl_var_typeK;
	e->val.var_type = var_type;
	return e;
}
/* =============== VAR_DECL_LIST END ================ */


/* =============== VAR_TYPE START =============== */
VAR_TYPE* make_VAR_TYPE(char* var_id, struct TYPE * type)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <var_type> at line: %d\n", lineno);
	VAR_TYPE * e = NEW(VAR_TYPE);
	e->lineno = lineno;
	e->id = var_id;
	e->type = type;
	return e;
}
/* =============== VAR_TYPE END ================ */


/* =============== BODY START =============== */
BODY* make_BODY(DECL_LIST * decl_list, STM_LIST * stm_list)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made <body> at line: %d\n", lineno);
	BODY * e = NEW(BODY);
	e->lineno = lineno;
	e->decl_list = decl_list;
	e->stm_list = stm_list;
	return e;
}
/* =============== BODY END ================ */


/* =============== DECL_LIST START =============== */
DECL_LIST* make_DECL_LIST_pair(DECL * head, DECL_LIST * tail)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <decl_list> at line: %d\n", lineno);
	DECL_LIST * e = NEW(DECL_LIST);
	e->lineno = lineno;
	e->kind = decl_list_pairK;
	e->pair.head = head;
	e->pair.tail = tail;
	return e;
}
DECL_LIST* make_DECL_LIST_empty()
{
	if (FALCON_DEBUG_PARSER)
		printf("made <decl_list> empty at line: %d\n", lineno);
	DECL_LIST * e = NEW(DECL_LIST);
	e->lineno = lineno;
	e->kind = decl_list_emptyK;
	return e;
}
/* =============== DECL_LIST END ================ */


/* =============== DECL START =============== */
DECL* make_DECL_assignment(char* id, TYPE * type)
{
	if (FALCON_DEBUG_PARSER)
		printf("made assignment <decl> at line: %d\n", lineno);
	DECL * e = NEW(DECL);
	e->lineno = lineno;
	e->kind = decl_assignK;
	e->decl_id = id;
	e->val.type = type;
	return e;
}
DECL* make_DECL_func(FUNCTION * func)
{
	if (FALCON_DEBUG_PARSER)
		printf("made func <decl> at line: %d\n", lineno);
	DECL * e = NEW(DECL);
	e->lineno = lineno;
	e->kind = decl_functionK;
	e->val.function = func;
	return e;
}
DECL* make_DECL_vars(VAR_DECL_LIST * var_list)
{
	if (FALCON_DEBUG_PARSER)
		printf("made var_decl_list <decl> at line: %d\n", lineno);
	DECL * e = NEW(DECL);
	e->lineno = lineno;
	e->kind = decl_varsK;
	e->val.var_decl_list = var_list;
	return e;
}
/* =============== DECL END ================ */


/* =============== STM_LIST START =============== */
STM_LIST* make_STM_LIST_statement(STM * stm)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a <stm_list> statement at line: %d\n", lineno);
	STM_LIST * e = NEW(STM_LIST);
	e->lineno = lineno;
	e->kind = stm_list_statementK;
	e->val.stm = stm;
	return e;
}

STM_LIST* make_STM_LIST_pair(STM * stm, STM_LIST * stmList)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a <stm_list> pair at line: %d\n", lineno);
	STM_LIST * e = NEW(STM_LIST);
	e->lineno = lineno;
	e->kind = stm_list_pairK;
	e->val.pair.head = stm;
	e->val.pair.tail = stmList;
	return e;
}
/* =============== STM_LIST END ================ */


/* =============== STM START =============== */
STM* make_STM_return(EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a return <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_returnK;
	e->val.exp = exp;
	return e;
}
STM* make_STM_write(EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a write <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_writeK;
	e->val.exp = exp;
	return e;
}
STM* make_STM_allocate(struct VARIABLE * var)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made an allocate <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_allocateK;
	e->val.var = var;
	return e;
}
STM* make_STM_allocate_of_length(struct VARIABLE * var, struct EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made an allocate of length <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_allocate_lengthK;
	e->val.allocate_length.var = var;
	e->val.allocate_length.exp = exp;
	return e;
}
STM* make_STM_assign(struct VARIABLE * var, struct EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made an assign <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_assignmentK;
	e->val.assignment.var = var;
	e->val.assignment.exp = exp;
	return e;
}

STM* make_STM_if(EXP * exp, STM * stm)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a if <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_ifK;
	e->val.if_stm.exp = exp;
	e->val.if_stm.stm = stm;
	return e;
}
STM* make_STM_if_else(EXP * exp, STM * stm_then, STM * stm_else)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a if-else <stm> at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_if_elseK;
	e->val.if_else_stm.exp = exp;
	e->val.if_else_stm.stm_then = stm_then;
	e->val.if_else_stm.stm_else = stm_else;
	return e;
}
STM* make_STM_list(STM_LIST * list)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a <stm> list at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_listK;
	e->val.list = list;
	return e;
}
STM* make_STM_while(EXP * exp, STM * stm)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a <stm> while at line: %d\n", lineno);
	STM * e = NEW(STM);
	e->lineno = lineno;
	e->kind = stm_whileK;
	e->val.while_stm.exp = exp;
	e->val.while_stm.stm = stm;
	return e;
}
/* =============== STM END ================ */


/* =============== VARIABLE START =============== */
VARIABLE* make_VARIABLE(char* id)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a <variable> with id: %s at line: %d\n", id, lineno);
	VARIABLE * e = NEW(VARIABLE);
	e->lineno = lineno;
	e->kind = variable_idK;
	e->val.id = id;
	return e;
}
VARIABLE* make_VARIABLE_array_access(struct VARIABLE * var, struct EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("Made a <variable> with array access at line: %d\n", lineno);
	VARIABLE * e = NEW(VARIABLE);
	e->lineno = lineno;
	e->kind = variable_arrayAccessK;
	e->val.array_access.var = var;
	e->val.array_access.exp = exp;
	return e;
}
VARIABLE* make_VARIABLE_access(struct VARIABLE * var, char* id)
{
	if (FALCON_DEBUG_PARSER)
		printf("=============== a Made a <variable> access with id: %s at line: %d\n", id, lineno);
	VARIABLE * e = NEW(VARIABLE);
	e->lineno = lineno;
	e->kind = variable_access_idK;
	e->val.access.id = id;
	e->val.access.var = var;
	return e;
}
/* =============== VARIABLE END ================ */


/* =============== EXP START =============== */
EXP* make_EXP_times(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made * at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_timesK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_div(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made / at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_divK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_plus(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made + at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_plusK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_minus(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made - at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_minusK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_equality(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made == at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_equalityK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_non_equality(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made != at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_non_equalityK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_greater(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made > at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_greaterK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_lesser(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made < at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_lesserK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_greater_equal(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made >= at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_greater_equalK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_lesser_equal(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <= at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_lesser_equalK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_and(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made && at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_andK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_or(EXP * left, EXP * right)
{
	if (FALCON_DEBUG_PARSER)
		printf("made || at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_orK;
	e->val.op.left = left;
	e->val.op.right = right;
	return e;
}
EXP* make_EXP_term(TERM * term)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <exp> term at line: %d\n", lineno);
	EXP * e = NEW(EXP);
	e->lineno = lineno;
	e->kind = exp_termK;
	e->val.term = term;
	return e;
}
/* =============== EXP END =============== */



/* =============== TERM START =============== */
TERM* make_TERM_variable(VARIABLE * var)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> variable at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_varK;
	e->val.var = var;
	return e;
}
TERM* make_TERM_id_act_list(char* id, struct ACT_LIST * act_list)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> id_act_list at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_id_act_listK;
	e->val.id_act_list.id = id;
	e->val.id_act_list.act_list = act_list;
	return e;
}
TERM* make_TERM_expression(EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> expression at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_parenthesis_expressionK;
	e->val.exp = exp;
	return e;
}
TERM* make_TERM_negate(TERM * term)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> negation at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_negationK;
	e->val.term = term;
	return e;
}
TERM* make_TERM_absolute(EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> absolute at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_absoluteK;
	e->val.exp = exp;
	return e;
}
TERM* make_TERM_num(int intconst)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> num at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_numK;
	e->val.num = intconst;
	return e;
}
TERM* make_TERM_true()
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> true at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_trueK;
	e->val.bool = 1;
	return e;
}
TERM* make_TERM_false()
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> false at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_falseK;
	e->val.bool = 0;
	return e;
}
TERM* make_TERM_null()
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> null at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_nullK;
	return e;
}
TERM* make_TERM_string(STRING * string)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <term> string at line: %d\n", lineno);
	TERM * e = NEW(TERM);
	e->lineno = lineno;
	e->kind = term_stringK;
	e->val.string = string;
	return e;
}

/* =============== TERM END =============== */


/* =============== ACT_LIST START =============== */
ACT_LIST* make_ACT_LIST_exp_list(struct EXP_LIST * exp_list)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <act_list> exp_list at line: %d\n", lineno);
	ACT_LIST * e = NEW(ACT_LIST);
	e->lineno = lineno;
	e->kind = act_list_exp_listK;
	e->exp_list = exp_list;
	return e;
}
ACT_LIST* make_ACT_LIST_empty()
{
	if (FALCON_DEBUG_PARSER)
		printf("made <act_list> empty at line: %d\n", lineno);
	ACT_LIST * e = NEW(ACT_LIST);
	e->lineno = lineno;
	e->kind = act_list_emptyK;
	return e;
}
/* =============== ACT_LIST END =============== */


/* =============== EXP_LIST START =============== */
EXP_LIST* make_EXP_LIST_expression(EXP * exp)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <exp_list> expression at line: %d\n", lineno);
	EXP_LIST * e = NEW(EXP_LIST);
	e->lineno = lineno;
	e->kind = exp_list_expressionK;
	e->val.exp = exp;
	return e;
}
EXP_LIST* make_EXP_LIST_pair(EXP * head, EXP_LIST * tail)
{
	if (FALCON_DEBUG_PARSER)
		printf("made <exp_list> pair at line: %d\n", lineno);
	EXP_LIST * e = NEW(EXP_LIST);
	e->lineno = lineno;
	e->kind = exp_list_pairK;
	e->val.pair.head = head;
	e->val.pair.tail = tail;
	return e;

}
/* =============== EXP_LIST END =============== */

