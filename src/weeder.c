#include "tree.h"
#include "weeder.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FUNCTION* weeder_global_function; //ptr to the currently scoped function. to keep track of it.
static unsigned int weeder_global_return_statements_encountered; //unsigned integer incremented for each function we're in.
static unsigned int weeder_global_error_count;
void weed_AST(BODY* body)
{
	weeder_global_function = 0;
	weeder_global_return_statements_encountered = 0;
	weeder_global_error_count = 0;
	weed_BODY(body);

	//stop immediately.
	//GHJ: The reason why we wait til the weeder is done,
	//is to give the user all the warnings and errors instead
	//of just stopping at the first error encountered.
	if (weeder_global_error_count > 0)
	{
		exit(1);
	}
}

void weed_STRING(STRING* s)
{
	if (s)
	{
		//no-op. to prevent gcc compilation warnings.
	}
}

void weed_FUNCTION(FUNCTION* f)
{
	weeder_global_function = f;
	weed_HEAD(f->func_head);
	weed_BODY(f->func_body);
	weed_TAIL(f->func_tail, f);
}

void weed_HEAD(HEAD* head)
{
	weed_PAR_DECL_LIST(head->params);
	weed_TYPE(head->return_type);
}

void weed_TAIL(TAIL* tail, FUNCTION* f)
{

	if (strcmp(f->func_head->function_id, tail->end_id) == 0)
	{
		//function is correct.
	}
	else
	{
		fprintf(stderr, "\x1b[37;1m"); //start print color white.
		fprintf(stderr, "%d: ", tail->lineno); //print  lineno
		fprintf(stderr, "\x1b[0m"); //reset color.
		fprintf(stderr, "\x1b[31;1m"); //start print color magenta.
		fprintf(stderr, "Error: "); //start print color magenta.
		fprintf(stderr, "\x1b[0m"); //reset color.
		fprintf(stderr, "In Function \x1b[37;1m'%s'\x1b[0m: Illegal function declaration, function head \x1b[37;1m'%s'\x1b[0m and function tail \x1b[37;1m'%s'\x1b[0m mismatch.\n",
			f->func_head->function_id, f->func_head->function_id, tail->end_id);
		weeder_global_error_count++;
	}

	//when we reach the tail, we know that we must see at least 1 return statement.
	//since, once we reach the tail, we have looked at the body node and its contents.
	if (weeder_global_return_statements_encountered == 0)
	{
		printf("\x1b[37;1m"); //start print color white.
		printf("%d: ", tail->lineno); //print lineno
		printf("\x1b[0m"); //reset color.
		printf("\x1b[36;1m"); //start print color magenta.
		printf("warning: "); //start print color magenta.
		printf("\x1b[0m"); //reset color.
		printf("In function \x1b[37;1m'%s'\x1b[0m: Function does not return a type.\n",
			f->func_head->function_id);

	}
	else
	{
		//function has at least one return type.
	}
	//reset function specific globals, since we're no longer in a function.
	weeder_global_return_statements_encountered = 0;
}

void weed_TYPE(TYPE * type)
{
	switch (type->kind)
	{
		case type_idK:
		break;
		case type_intK:
		break;
		case type_boolK:
		break;
		case type_arrayK:
		weed_TYPE(type->val.type);
		break;
		case type_recordK:
		weed_VAR_DECL_LIST(type->val.var_decl_list);
		break;
		case type_stringK:
		break;
		default:  break;
	}
}


void weed_PAR_DECL_LIST(PAR_DECL_LIST * par_decl_list)
{
	switch (par_decl_list->kind)
	{
		case par_decl_list_var_decl_listK:
		weed_VAR_DECL_LIST(par_decl_list->val.var_decl_list);
		break;
		case par_decl_list_emptyK:
		break;
		default:  break;;
	}
}
void weed_VAR_DECL_LIST(VAR_DECL_LIST * var_decl_list)
{
	switch (var_decl_list->kind)
	{
		case var_decl_list_pairK:
		weed_VAR_TYPE(var_decl_list->val.pair.head);
		weed_VAR_DECL_LIST(var_decl_list->val.pair.tail);
		break;
		case var_decl_var_typeK:
		weed_VAR_TYPE(var_decl_list->val.var_type);
		break;
		default:  break;
	}
}

void weed_VAR_TYPE(VAR_TYPE * var_type)
{
	weed_TYPE(var_type->type);
}


void weed_BODY(BODY * body)
{
	weed_DECL_LIST(body->decl_list);
	weed_STM_LIST(body->stm_list);
}

void weed_DECL_LIST(DECL_LIST * decl_list)
{
	switch (decl_list->kind)
	{
		case decl_list_pairK:
		weed_DECL(decl_list->pair.head);
		weed_DECL_LIST(decl_list->pair.tail);
		break;
		case decl_list_emptyK:
		break;
		default:  break;
	}
}


void weed_DECL(DECL * decl)
{
	switch (decl->kind)
	{
		case decl_assignK:
		weed_TYPE(decl->val.type);
		break;
		case decl_functionK:
		weed_FUNCTION(decl->val.function);
		break;
		case decl_varsK:
		weed_VAR_DECL_LIST(decl->val.var_decl_list);
		break;
		default:  break;
	}
}

void weed_STM_LIST(STM_LIST * stm_list)
{
	switch (stm_list->kind)
	{
		case stm_list_pairK:
		weed_STM(stm_list->val.pair.head);
		weed_STM_LIST(stm_list->val.pair.tail);
		break;
		case stm_list_statementK:
		weed_STM(stm_list->val.stm); //expect one.
		break;
		default:  break;
	}
}


void weed_STM(STM * stm)
{
	switch (stm->kind)
	{
		case stm_returnK:
		weeder_global_return_statements_encountered++;
		weed_EXP(stm->val.exp);
		break;
		case stm_writeK:
		weed_EXP(stm->val.exp);
		break;
		case stm_allocateK:
		weed_VARIABLE(stm->val.var);
		break;
		case stm_allocate_lengthK:
		weed_VARIABLE(stm->val.allocate_length.var);
		weed_EXP(stm->val.allocate_length.exp);
		break;
		case stm_assignmentK:
		weed_VARIABLE(stm->val.assignment.var);
		weed_EXP(stm->val.assignment.exp);
		break;
		case stm_ifK:
		weed_EXP(stm->val.if_stm.exp);
		weed_STM(stm->val.if_stm.stm);
		break;
		case stm_if_elseK:
		weed_EXP(stm->val.if_else_stm.exp);
		weed_STM(stm->val.if_else_stm.stm_then);
		weed_STM(stm->val.if_else_stm.stm_else);
		break;

		case stm_whileK:
		weed_EXP(stm->val.while_stm.exp);
		weed_STM(stm->val.while_stm.stm);
		break;

		case stm_listK:
		weed_STM_LIST(stm->val.list);
		break;
		default:  break;
	}
}


void weed_VARIABLE(VARIABLE * var)
{
	switch (var->kind)
	{
		case variable_idK:
		break;
		case variable_arrayAccessK:
		weed_VARIABLE(var->val.array_access.var);
		weed_EXP(var->val.array_access.exp);
		break;
		case variable_access_idK:
		weed_VARIABLE(var->val.access.var);
		break;
		default: break;
	}
}

void weed_EXP(EXP * exp)
{
	switch (exp->kind)
	{
		case exp_timesK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_divK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_plusK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_minusK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_equalityK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_non_equalityK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_greaterK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_lesserK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_greater_equalK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_lesser_equalK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_andK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_orK:
		weed_EXP(exp->val.op.left);
		weed_EXP(exp->val.op.right);
		break;
		case exp_termK:
		weed_TERM(exp->val.term);
		break;

		default:break;
	}
}

void weed_TERM(TERM * term)
{
	switch (term->kind)
	{
		case term_varK:
		weed_VARIABLE(term->val.var);
		break;
		case term_id_act_listK:
		weed_ACT_LIST(term->val.id_act_list.act_list);
		break;
		case term_parenthesis_expressionK:
		weed_EXP(term->val.exp);
		break;
		case term_negationK:
		weed_TERM(term->val.term);
		break;
		case term_absoluteK:
		weed_EXP(term->val.exp);
		break;
		case term_numK:
		break;
		case term_trueK:
		break;
		case term_falseK:
		break;
		case term_nullK:
		break;
		case term_stringK:
		weed_STRING(term->val.string);
		break;
		default:  break;
	}
}


void weed_ACT_LIST(ACT_LIST * act_list)
{
	switch (act_list->kind)
	{
		case act_list_exp_listK:
		weed_EXP_LIST(act_list->exp_list);
		break;
		case act_list_emptyK:
		break;
		default:  break;
	}
}


void weed_EXP_LIST(EXP_LIST * exp_list)
{
	switch (exp_list->kind)
	{
		case exp_list_pairK:
		weed_EXP(exp_list->val.pair.head);
		weed_EXP_LIST(exp_list->val.pair.tail);
		break;
		case exp_list_expressionK:
		weed_EXP(exp_list->val.exp);
		break;
		default:  break;
	}
}

