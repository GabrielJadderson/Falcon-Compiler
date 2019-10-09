%{
#include <stdio.h>
#include "tree.h"

extern char *yytext;
extern BODY* AST;

void yyerror() {
  printf("syntax error before %s\n", yytext);
}

int yylex(); //to prevent implicit declaration warnings.


%}

%union {
   int intconst;
   char *stringconst;
   char* dynamic_string;	//for holding actual strings in our language like "hello world" (without quotes).
   struct STRING* string;
   struct FUNCTION* function;
   struct HEAD* head;
   struct TAIL* tail;
   struct TYPE* type;
   struct PAR_DECL_LIST* par_decl_list;
   struct VAR_DECL_LIST* var_decl_list;
   struct VAR_TYPE* var_type;
   struct BODY* body;
   struct DECL_LIST* decl_list;
   struct DECL* declaration;
   struct STM_LIST* statement_list;
   struct STM* statement;
   struct VARIABLE* variable;
   struct EXP *exp;
   struct TERM* term;
   struct ACT_LIST* act_list;
   struct EXP_LIST* exp_list;
}

%token <intconst> tINTCONST
%token <stringconst> tIDENTIFIER
%token <dynamic_string> tSTRING
%token tEQUALITY
%token tLESSER_EQUAL
%token tGREATER_EQUAL
%token tNON_EQUALITY
%token tAND
%token tOR
%token tRETURN
%token tWRITE
%token tIF
%token tELSE
%token tTHEN
%token tALLOCATE
%token tWHILE
%token tDO
%token tBOOL
%token tARRAY_OF
%token tTRUE
%token tFALSE
%token tNULL
%token tTYPE
%token tINT
%token tFUNC
%token tEND
%token tVAR
%token tOF_LENGTH
%token tRECORD_OF
%token tSTRING_TOKEN

%token tPLUS_EQUAL
%token tMINUS_EQUAL
%token tTIMES_EQUAL
%token tDIVIDE_EQUAL
%token tPLUS_PLUS_INCREMENT
%token tMINUS_MINUS_DECREMENT

%type <string> string

%type <function> function
%type <head> head
%type <tail> tail
%type <type> type
%type <par_decl_list> par_decl_list
%type <var_decl_list> var_decl_list
%type <var_type> var_type

%type <body> program body

%type <decl_list> decl_list
%type <declaration> declaration
%type <statement_list> statement_list
%type <statement> statement
%type <variable> variable
%type <exp> exp
%type <term> term
%type <act_list> act_list
%type <exp_list> exp_list

%expect 2

%start program

%left tOR
%left tAND
%left tEQUALITY tNON_EQUALITY tPLUS_EQUAL tMINUS_EQUAL tTIMES_EQUAL tDIVIDE_EQUAL tAND_EQUAL tOR_EQUAL tPLUS_PLUS_INCREMENT tMINUS_MINUS_DECREMENT
%left '<' '>' tLESSER_EQUAL tGREATER_EQUAL
%left '+' '-'
%left '*' '/'

%%
program: body
         { AST = $1;}
;


string : tSTRING
	{
		$$ = make_STRING($1);
	}
	| tSTRING '+' tSTRING
	{
		$$ = make_STRING_concatenation($1, $3);
	}
;


function : head body tail
	{
		$$ = make_FUNC($1, $2, $3);
	}
;


head : tFUNC tIDENTIFIER '(' par_decl_list ')' ':' type
	{
		$$ = make_HEAD($2, $4, $7);
	}
;

tail : tEND tIDENTIFIER
	{
		$$ = make_TAIL($2);
	}
;

type : tIDENTIFIER
	{
		$$ = make_TYPE_id($1);
	}
	| tINT
	{
		$$ = make_TYPE_int();
	}
	| tBOOL
	{
		$$= make_TYPE_bool();
	}
	| tSTRING_TOKEN
	{
		$$ = make_TYPE_string();
	}
	| tARRAY_OF type
	{
		$$ = make_TYPE_array_of($2);
	}
	| tRECORD_OF '{' var_decl_list '}'
	{
		$$ = make_TYPE_record($3);
	}
;

par_decl_list : var_decl_list
	{
		$$ = make_PAR_DECL_LIST($1);
	}
	| %empty
	{
		$$ = make_PAR_DECL_LIST_empty();
	}
;


var_decl_list : var_type ',' var_decl_list
	{
		$$ = make_VAR_DECL_LIST_pair($1, $3);
	}
	| var_type
	{
		$$ = make_VAR_DECL_LIST_last($1);
	}
;

var_type : tIDENTIFIER ':' type
	{
		$$ = make_VAR_TYPE($1, $3);
	}
;

body : decl_list statement_list
	{
		$$ = make_BODY($1,$2);
	}
;

decl_list : declaration decl_list
	{
		$$ = make_DECL_LIST_pair($1, $2);
	}
	| %empty
	{
		$$ = make_DECL_LIST_empty();
	}
;


declaration : tTYPE tIDENTIFIER '=' type ';'
	{
		$$ = make_DECL_assignment($2, $4);
	}
	| function
	{
		$$ = make_DECL_func($1);
	}
	| tVAR var_decl_list ';'
	{
		$$ = make_DECL_vars($2);
	}
;

statement_list : statement
	{
		$$ = make_STM_LIST_statement($1);
	}
	| statement statement_list
	{
		$$ = make_STM_LIST_pair($1, $2);
	}
;


statement : tRETURN exp ';'
	{
		$$ = make_STM_return($2);
	}
	| tWRITE exp ';'
	{
		$$ = make_STM_write($2);
	}
	| tALLOCATE variable ';'
	{
		$$ = make_STM_allocate($2);
	}
	| tALLOCATE variable tOF_LENGTH exp ';'
	{
		$$ = make_STM_allocate_of_length($2, $4);
	}
	| variable '=' exp ';'
	{
		$$ = make_STM_assign($1, $3);
	}
	| variable tPLUS_EQUAL exp ';'
	{
		$$ = make_STM_assign($1, make_EXP_plus(make_EXP_term(make_TERM_variable($1)), $3)); //variant 1
	}
	| variable tMINUS_EQUAL exp ';'
	{
		$$ = make_STM_assign($1, make_EXP_minus(make_EXP_term(make_TERM_variable($1)), $3)); //variant 2
	}
	| variable tTIMES_EQUAL exp ';'
	{
		$$ = make_STM_assign($1, make_EXP_times(make_EXP_term(make_TERM_variable($1)), $3)); //variant 2
	}
	| variable tDIVIDE_EQUAL exp ';'
	{
		$$ = make_STM_assign($1, make_EXP_div(make_EXP_term(make_TERM_variable($1)), $3)); //variant 4
	}
	| variable tPLUS_PLUS_INCREMENT ';'
	{
		$$ = make_STM_assign($1, make_EXP_plus(make_EXP_term(make_TERM_variable($1)), make_EXP_term(make_TERM_num(1)))); //variant 1
	}
	| variable tMINUS_MINUS_DECREMENT ';'
	{
		$$ = make_STM_assign($1, make_EXP_minus(make_EXP_term(make_TERM_variable($1)), make_EXP_term(make_TERM_num(1)))); //variant 1
	}
	| tIF exp tTHEN statement
	{
		$$ =  make_STM_if($2, $4);
	}
	| tIF exp tTHEN statement tELSE statement
	{
		$$ = make_STM_if_else($2, $4, $6);
	}
	| tWHILE exp tDO statement
	{
		$$ = make_STM_while($2, $4);
	}
	|'{' statement_list '}'
	{
		$$ = make_STM_list($2);
	}
;


variable : tIDENTIFIER
	{
		$$ = make_VARIABLE($1);
	}
	| variable '[' exp ']'
	{
		$$ =  make_VARIABLE_array_access($1, $3);
	}
	| variable '.' tIDENTIFIER
	{
		$$ = make_VARIABLE_access($1, $3);
	}
;


exp : exp '*' exp
	{
		$$ = make_EXP_times($1,$3);
	}
	| exp '/' exp
	{
		$$ = make_EXP_div($1,$3);
	}
	| exp '+' exp
	{
		$$ = make_EXP_plus($1,$3);
	}
	| exp '-' exp
	{
		$$ = make_EXP_minus($1,$3);
	}
	| exp tEQUALITY exp
	{
		$$ = make_EXP_equality($1, $3);
	}
	| exp tOR exp
	{
		$$ = make_EXP_or($1, $3);
	}
	| exp tAND exp
	{
		$$ = make_EXP_and($1, $3);
	}
	| exp tNON_EQUALITY exp
	{
		$$ = make_EXP_non_equality($1, $3);
	}
	| exp tGREATER_EQUAL exp
	{
		$$ = make_EXP_greater_equal($1, $3);
	}
	| exp tLESSER_EQUAL exp
	{
		$$ = make_EXP_lesser_equal($1, $3);
	}
	| exp '>' exp
	{
		$$ = make_EXP_greater($1, $3);
	}
	| exp '<' exp
	{
		$$ = make_EXP_lesser($1, $3);
	}
	| term
	{
		$$ = make_EXP_term($1);
	}
;


term : variable
	{
		$$ = make_TERM_variable($1);
	}
	| tIDENTIFIER '(' act_list ')'
	{
		$$ = make_TERM_id_act_list($1, $3);
	}
	| '(' exp ')'
	{
		$$ = make_TERM_expression($2);
	}
	| '!' term
	{
		$$ = make_TERM_negate($2);
	}
	| '|' exp '|'
	{
		$$ = make_TERM_absolute($2);
	}
	| tINTCONST
	{
		$$ = make_TERM_num($1);
	}
	| tTRUE
	{
		$$ = make_TERM_true();
	}
	| tFALSE
	{
		$$ = make_TERM_false();
	}
	| tNULL
	{
		$$ = make_TERM_null();
	}
	| string
	{
		$$ = make_TERM_string($1);
	}
;


act_list : exp_list
	{
		$$ = make_ACT_LIST_exp_list($1);
	}
	| %empty
	{
		$$ = make_ACT_LIST_empty();
	}
;


exp_list : exp
	{
		$$ = make_EXP_LIST_expression($1);
	}
	| exp ',' exp_list
	{
		$$ = make_EXP_LIST_pair($1, $3);
	}
;


%%
