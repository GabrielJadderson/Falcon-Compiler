#include <stdio.h>
#include "pretty.h"
#include "y.tab.h"


extern char FALCON_DEBUG_PARSER;
extern char FALCON_DEBUG_AST;

void prettyAST(BODY* AST)
{

	if (FALCON_DEBUG_AST)
	{
		printf("\\begin{figure}[H] \n");
		printf("\\resizebox{0.9\\textwidth}{!}{ \n");
		printf("\\Tree \n");

	}

	if (FALCON_DEBUG_PARSER || FALCON_DEBUG_AST)
		prettyBODY(AST);

	if (FALCON_DEBUG_AST)
	{
		printf(" } \n");
		printf("\\caption{AST for a the Simple program.}\n");
		printf("\\label{ast:1}\n");
		printf("\\end{figure}\n");
	}
}

void prettySTRING(STRING* s)
{

	if (!FALCON_DEBUG_AST)
	{
		printf(" <string>\"%s\"</string> ", s->str);
	}
	else
	{
		printf("[.string \n");
		printf(" \"%s\" ", s->str);
		printf(" ] \n");
	}
}

void prettyFUNCTION(FUNCTION* f)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <function>");
		prettyHEAD(f->func_head);
		prettyBODY(f->func_body);
		prettyTAIL(f->func_tail);
		printf("</function> ");
	}
	else
	{
		printf("[.function \n");
		prettyHEAD(f->func_head);
		prettyBODY(f->func_body);
		prettyTAIL(f->func_tail);
		printf(" ] \n");
	}

}

void prettyHEAD(HEAD* head)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <head>func %s", head->function_id);
		prettyPAR_DECL_LIST(head->params);
		printf(" : ");
		prettyTYPE(head->return_type);
		printf("</head> ");
	}
	else
	{
		printf("[.head \n");
		printf(" func %s ", head->function_id);
		prettyPAR_DECL_LIST(head->params);
		printf(" : ");
		prettyTYPE(head->return_type);
		printf(" ] \n");
	}

}

void prettyTAIL(TAIL* tail)
{
	if (!FALCON_DEBUG_AST)
	{
		printf("<tail>end %s</tail> ", tail->end_id);
	}
	else
	{
		printf("[.tail \n");
		printf(" end %s ", tail->end_id);
		printf(" ] \n");
	}
}

void prettyTYPE(TYPE* type)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <type>");
		switch (type->kind)
		{
			case type_idK:
			printf("  %s ", type->val.type_id);
			break;
			case type_intK:
			printf(" int ");
			break;
			case type_boolK:
			printf(" bool ");
			break;
			case type_arrayK:
			printf(" array of ");
			prettyTYPE(type->val.type);
			break;
			case type_recordK:
			printf(" record of ");
			prettyVAR_DECL_LIST(type->val.var_decl_list);
			break;
			case type_stringK:
			printf(" string");
			break;
			default: printf("\n default <type> \n"); break;
		}
		printf("</type> ");
	}
	else
	{
		printf("[.type \n");
		switch (type->kind)
		{
			case type_idK:
			printf("  %s ", type->val.type_id);
			break;
			case type_intK:
			printf(" int ");
			break;
			case type_boolK:
			printf(" bool ");
			break;
			case type_arrayK:
			printf(" array of ");
			prettyTYPE(type->val.type);
			break;
			case type_recordK:
			printf(" record of ");
			prettyVAR_DECL_LIST(type->val.var_decl_list);
			break;
			case type_stringK:
			printf(" string");
			break;
			default: printf("  default type \n"); break;
		}
		printf(" ] \n");
	}

}


void prettyPAR_DECL_LIST(PAR_DECL_LIST* par_decl_list)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <par_decl_list>");
		switch (par_decl_list->kind)
		{
			case par_decl_list_var_decl_listK:
			prettyVAR_DECL_LIST(par_decl_list->val.var_decl_list);
			break;
			case par_decl_list_emptyK:
			printf(" empty ");
			break;
			default: printf("\n default <par_decl_list> \n"); break;;
		}
		printf("</par_decl_list> ");
	}
	else
	{
		printf("[.par\\_decl\\_list \n");
		switch (par_decl_list->kind)
		{
			case par_decl_list_var_decl_listK:
			prettyVAR_DECL_LIST(par_decl_list->val.var_decl_list);
			break;
			case par_decl_list_emptyK:
			printf(" empty ");
			break;
			default: printf(" default par\\_decl\\_list \n"); break;;
		}
		printf(" ] \n");
	}
}
void prettyVAR_DECL_LIST(VAR_DECL_LIST* var_decl_list)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <var_decl_list>");
		switch (var_decl_list->kind)
		{
			case var_decl_list_pairK:
			prettyVAR_TYPE(var_decl_list->val.pair.head);
			printf(" , ");
			prettyVAR_DECL_LIST(var_decl_list->val.pair.tail);
			break;
			case var_decl_var_typeK:
			prettyVAR_TYPE(var_decl_list->val.var_type);
			break;
			default: printf("\n default <var_decl_list> \n"); break;
		}
		printf("</var_decl_list> ");
	}
	else
	{
		printf("[.var\\_decl\\_list \n");
		switch (var_decl_list->kind)
		{
			case var_decl_list_pairK:
			prettyVAR_TYPE(var_decl_list->val.pair.head);
			printf(" , ");
			prettyVAR_DECL_LIST(var_decl_list->val.pair.tail);
			break;
			case var_decl_var_typeK:
			prettyVAR_TYPE(var_decl_list->val.var_type);
			break;
			default: printf(" default var\\_decl\\_list \n"); break;
		}
		printf(" ] \n");
	}

}

void prettyVAR_TYPE(VAR_TYPE* var_type)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <var_type>");
		printf("%s : ", var_type->id);
		prettyTYPE(var_type->type);
		printf("</var_type> ");
	}
	else
	{
		printf("[.var\\_type ");
		printf(" %s : ", var_type->id);
		prettyTYPE(var_type->type);
		printf(" ] \n");
	}

}


void prettyBODY(BODY* body)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <body>");
		printf("\n");
		prettyDECL_LIST(body->decl_list);
		printf("\n");
		prettySTM_LIST(body->stm_list);
		printf("\n</body>");
	}
	else
	{
		printf("[.body \n");
		prettyDECL_LIST(body->decl_list);
		prettySTM_LIST(body->stm_list);
		printf(" ] \n");
	}
}

void prettyDECL_LIST(DECL_LIST* decl_list)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <decl_list>");
		switch (decl_list->kind)
		{
			case decl_list_pairK:
			prettyDECL(decl_list->pair.head);
			prettyDECL_LIST(decl_list->pair.tail);
			break;
			case decl_list_emptyK:
			printf(" empty ");
			break;
			default: printf("\n default <decl_list> \n"); break;
		}
		printf("</decl_list>");
	}
	else
	{
		printf("[.decl\\_list \n");

		switch (decl_list->kind)
		{
			case decl_list_pairK:
			prettyDECL(decl_list->pair.head);
			prettyDECL_LIST(decl_list->pair.tail);
			break;
			case decl_list_emptyK:
			printf(" (empty) ");
			break;
			default: printf(" default decl\\_list \n"); break;
		}

		printf(" ] \n");
	}

}

void prettyDECL(DECL* decl)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <decl>");
		switch (decl->kind)
		{
			case decl_assignK:
			printf("type %s = ", decl->decl_id);
			prettyTYPE(decl->val.type);
			break;
			case decl_functionK:
			prettyFUNCTION(decl->val.function);
			break;
			case decl_varsK:
			prettyVAR_DECL_LIST(decl->val.var_decl_list);
			break;
			default: printf("\n default <decl> \n"); break;
		}
		printf("</decl> ");
	}
	else
	{
		printf("[.declaration \n");
		switch (decl->kind)
		{
			case decl_assignK:
			printf(" type %s = ", decl->decl_id);
			prettyTYPE(decl->val.type);
			break;
			case decl_functionK:
			prettyFUNCTION(decl->val.function);
			break;
			case decl_varsK:
			prettyVAR_DECL_LIST(decl->val.var_decl_list);
			break;
			default: printf(" default declaration \n"); break;
		}
		printf(" ] \n");
	}

}

void prettySTM_LIST(STM_LIST* stm_list)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <stm_list>");
		switch (stm_list->kind)
		{
			case stm_list_pairK:
			prettySTM(stm_list->val.pair.head);
			prettySTM_LIST(stm_list->val.pair.tail);
			break;
			case stm_list_statementK:
			prettySTM(stm_list->val.stm);
			break;
			default: printf("\n default <stm_list> \n"); break;
		}
		printf("\n</stm_list> ");
	}
	else
	{
		printf("[.stm\\_list \n");
		switch (stm_list->kind)
		{
			case stm_list_pairK:
			prettySTM(stm_list->val.pair.head);
			prettySTM_LIST(stm_list->val.pair.tail);
			break;
			case stm_list_statementK:
			prettySTM(stm_list->val.stm);
			break;
			default: printf("\n default <stm\\_list> \n"); break;
		}
		printf(" ] \n");
	}

}


void prettySTM(STM* stm)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <stm>");
		switch (stm->kind)
		{

			case stm_returnK:
			printf(" return ");
			prettyEXP(stm->val.exp);
			printf("; ");
			break;
			case stm_writeK:
			printf(" write ");
			prettyEXP(stm->val.exp);
			printf("; ");
			break;
			case stm_allocateK:
			printf(" allocate ");
			prettyVARIABLE(stm->val.var);
			printf("; ");
			break;
			case stm_allocate_lengthK:
			printf(" allocate ");
			prettyVARIABLE(stm->val.allocate_length.var);
			printf(" of length ");
			prettyEXP(stm->val.allocate_length.exp);
			printf("; ");
			break;
			case stm_assignmentK:
			prettyVARIABLE(stm->val.assignment.var);
			printf(" = ");
			prettyEXP(stm->val.assignment.exp);
			printf("; ");
			break;
			case stm_ifK:
			printf(" if ");
			prettyEXP(stm->val.if_stm.exp);
			printf(" then ");
			prettySTM(stm->val.if_stm.stm);
			break;
			case stm_if_elseK:
			printf("if ");
			prettyEXP(stm->val.if_else_stm.exp);
			printf(" then ");
			prettySTM(stm->val.if_else_stm.stm_then);
			printf(" else ");
			prettySTM(stm->val.if_else_stm.stm_else);
			break;

			case stm_whileK:
			printf(" while ");
			prettyEXP(stm->val.while_stm.exp);
			printf(" do ");
			prettySTM(stm->val.while_stm.stm);
			break;

			case stm_listK:
			printf("{ ");
			prettySTM_LIST(stm->val.list);
			printf(" }");
			break;
			default: printf("\n default <stm> \n"); break;
		}
		printf("</stm> ");
	}
	else
	{
		printf("[.stm \n");
		switch (stm->kind)
		{
			case stm_returnK:
			printf(" return ");
			prettyEXP(stm->val.exp);
			printf(" ; ");
			break;
			case stm_writeK:
			printf(" write ");
			prettyEXP(stm->val.exp);
			printf(" ; ");
			break;
			case stm_allocateK:
			printf(" allocate ");
			prettyVARIABLE(stm->val.var);
			printf(" ; ");
			break;
			case stm_allocate_lengthK:
			printf(" allocate ");
			prettyVARIABLE(stm->val.allocate_length.var);
			printf(" of length ");
			prettyEXP(stm->val.allocate_length.exp);
			printf(" ; ");
			break;
			case stm_assignmentK:
			prettyVARIABLE(stm->val.assignment.var);
			printf(" $=$ ");
			prettyEXP(stm->val.assignment.exp);
			printf(" ; ");
			break;
			case stm_ifK:
			printf(" if ");
			prettyEXP(stm->val.if_stm.exp);
			printf(" then ");
			prettySTM(stm->val.if_stm.stm);
			break;
			case stm_if_elseK:
			printf(" if ");
			prettyEXP(stm->val.if_else_stm.exp);
			printf(" then ");
			prettySTM(stm->val.if_else_stm.stm_then);
			printf(" else ");
			prettySTM(stm->val.if_else_stm.stm_else);
			break;
			case stm_whileK:
			printf(" while ");
			prettyEXP(stm->val.while_stm.exp);
			printf(" do ");
			prettySTM(stm->val.while_stm.stm);
			break;
			case stm_listK:
			printf(" \\{ ");
			prettySTM_LIST(stm->val.list);
			printf(" \\} ");
			break;
			default: printf("\n default stm \n"); break;
		}
		printf(" ] \n");
	}

}


void prettyVARIABLE(VARIABLE* var)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <variable>");
		switch (var->kind)
		{
			case variable_idK:
			printf("%s", var->val.id);
			break;
			case variable_arrayAccessK:
			prettyVARIABLE(var->val.array_access.var);
			printf(" [");
			prettyEXP(var->val.array_access.exp);
			printf("]");
			break;
			case variable_access_idK:
			prettyVARIABLE(var->val.access.var);
			printf(".%s", var->val.access.id);
			break;
			default:
			printf("\n default <variable> at: %d \n", var->lineno);
			break;
		}
		printf("</variable> ");
	}
	else
	{
		printf("[.variable \n");
		switch (var->kind)
		{
			case variable_idK:
			printf(" %s ", var->val.id);
			break;
			case variable_arrayAccessK:
			prettyVARIABLE(var->val.array_access.var);
			printf(" \\text{[} ");
			prettyEXP(var->val.array_access.exp);
			printf(" \\text{]} ");
			break;
			case variable_access_idK:
			prettyVARIABLE(var->val.access.var);
			printf(" .%s ", var->val.access.id);
			break;
			default:
			printf(" default variable at: %d \n", var->lineno);
			break;
		}
		printf(" ] \n");
	}

}

void prettyEXP(EXP* exp)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <exp>");
		switch (exp->kind)
		{
			case exp_timesK:
			prettyEXP(exp->val.op.left);
			printf("*");
			prettyEXP(exp->val.op.right);
			break;
			case exp_divK:
			prettyEXP(exp->val.op.left);
			printf("/");
			prettyEXP(exp->val.op.right);
			break;
			case exp_plusK:
			prettyEXP(exp->val.op.left);
			printf("+");
			prettyEXP(exp->val.op.right);
			break;
			case exp_minusK:
			prettyEXP(exp->val.op.left);
			printf("-");
			prettyEXP(exp->val.op.right);
			break;
			case exp_equalityK:
			prettyEXP(exp->val.op.left);
			printf("=");
			prettyEXP(exp->val.op.right);
			break;
			case exp_non_equalityK:
			prettyEXP(exp->val.op.left);
			printf("!=");
			prettyEXP(exp->val.op.right);
			break;
			case exp_greaterK:
			prettyEXP(exp->val.op.left);
			printf(">");
			prettyEXP(exp->val.op.right);
			break;
			case exp_lesserK:
			prettyEXP(exp->val.op.left);
			printf("<");
			prettyEXP(exp->val.op.right);
			break;
			case exp_greater_equalK:
			prettyEXP(exp->val.op.left);
			printf(">=");
			prettyEXP(exp->val.op.right);
			break;
			case exp_lesser_equalK:
			prettyEXP(exp->val.op.left);
			printf("<=");
			prettyEXP(exp->val.op.right);
			break;
			case exp_andK:
			prettyEXP(exp->val.op.left);
			printf("&&");
			prettyEXP(exp->val.op.right);
			break;
			case exp_orK:
			prettyEXP(exp->val.op.left);
			printf("||");
			prettyEXP(exp->val.op.right);
			break;
			case exp_termK:
			prettyTERM(exp->val.term);
			break;

			default: printf("\n default <exp> \n"); break;
		}
		printf("</exp> ");
	}
	else
	{
		printf("[.exp \n");
		switch (exp->kind)
		{
			case exp_timesK:
			prettyEXP(exp->val.op.left);
			printf(" $*$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_divK:
			prettyEXP(exp->val.op.left);
			printf(" $/$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_plusK:
			prettyEXP(exp->val.op.left);
			printf(" $+$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_minusK:
			prettyEXP(exp->val.op.left);
			printf(" $-$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_equalityK:
			prettyEXP(exp->val.op.left);
			printf(" $=$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_non_equalityK:
			prettyEXP(exp->val.op.left);
			printf(" $\\neq$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_greaterK:
			prettyEXP(exp->val.op.left);
			printf(" $>$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_lesserK:
			prettyEXP(exp->val.op.left);
			printf(" $<$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_greater_equalK:
			prettyEXP(exp->val.op.left);
			printf(" $\\geq$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_lesser_equalK:
			prettyEXP(exp->val.op.left);
			printf(" $\\leq$ ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_andK:
			prettyEXP(exp->val.op.left);
			printf(" \\&\\& ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_orK:
			prettyEXP(exp->val.op.left);
			printf(" || ");
			prettyEXP(exp->val.op.right);
			break;
			case exp_termK:
			prettyTERM(exp->val.term);
			break;

			default: printf(" default exp \n"); break;
		}
		printf(" ] \n");
	}
}

void prettyTERM(TERM* term)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <term>");
		switch (term->kind)
		{
			case term_varK:
			prettyVARIABLE(term->val.var);
			break;
			case term_id_act_listK:
			printf(" %s ", term->val.id_act_list.id);
			prettyACT_LIST(term->val.id_act_list.act_list);
			break;
			case term_parenthesis_expressionK:
			prettyEXP(term->val.exp);
			break;
			case term_negationK:
			printf("!");
			prettyTERM(term->val.term);
			break;
			case term_absoluteK:
			printf("|");
			prettyEXP(term->val.exp);
			printf("|");
			break;
			case term_numK:
			printf("%d", term->val.num);
			break;
			case term_trueK:
			printf("true");
			break;
			case term_falseK:
			printf("false");
			break;
			case term_nullK:
			printf("null");
			break;
			case term_stringK:
			prettySTRING(term->val.string);
			break;
			default: printf("\n default <term> \n"); break;
		}
		printf("</term> ");
	}
	else
	{
		printf("[.term \n");
		switch (term->kind)
		{
			case term_varK:
			prettyVARIABLE(term->val.var);
			break;
			case term_id_act_listK:
			printf(" %s ", term->val.id_act_list.id);
			prettyACT_LIST(term->val.id_act_list.act_list);
			break;
			case term_parenthesis_expressionK:
			prettyEXP(term->val.exp);
			break;
			case term_negationK:
			printf(" ! ");
			prettyTERM(term->val.term);
			break;
			case term_absoluteK:
			printf(" | ");
			prettyEXP(term->val.exp);
			printf(" | ");
			break;
			case term_numK:
			printf(" %d ", term->val.num);
			break;
			case term_trueK:
			printf(" true ");
			break;
			case term_falseK:
			printf(" false ");
			break;
			case term_nullK:
			printf(" null ");
			break;
			case term_stringK:
			prettySTRING(term->val.string);
			break;
			default: printf(" default term \n"); break;
		}
		printf(" ] \n");
	}

}


void prettyACT_LIST(ACT_LIST* act_list)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <act_list>");
		switch (act_list->kind)
		{
			case act_list_exp_listK:
			prettyEXP_LIST(act_list->exp_list);
			break;
			case act_list_emptyK:
			printf(" empty ");
			break;
			default: printf("\ndefault <act_list> \n"); break;
		}
		printf("</act_list> ");
	}
	else
	{
		printf("[.act\\_list ");
		switch (act_list->kind)
		{
			case act_list_exp_listK:
			prettyEXP_LIST(act_list->exp_list);
			break;
			case act_list_emptyK:
			printf(" empty ");
			break;
			default: printf("\ndefault <act\\_list \n"); break;
		}
		printf(" ] \n");
	}

}


void prettyEXP_LIST(EXP_LIST * exp_list)
{
	if (!FALCON_DEBUG_AST)
	{
		printf(" <exp_list>");
		switch (exp_list->kind)
		{
			case exp_list_pairK:
			prettyEXP(exp_list->val.pair.head);
			printf(" , ");
			prettyEXP_LIST(exp_list->val.pair.tail);
			break;
			case exp_list_expressionK:
			prettyEXP(exp_list->val.exp);
			break;
			default: printf("\n default <exp_list> \n"); break;
		}
		printf("</exp_list> ");
	}
	else
	{
		printf("[.exp\\_list ");
		switch (exp_list->kind)
		{
			case exp_list_pairK:
			prettyEXP(exp_list->val.pair.head);
			printf(" , ");
			prettyEXP_LIST(exp_list->val.pair.tail);
			break;
			case exp_list_expressionK:
			prettyEXP(exp_list->val.exp);
			break;
			default: printf(" default exp\\_list \n"); break;
		}
		printf(" ] \n");
	}

}
