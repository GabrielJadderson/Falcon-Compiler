#pragma once
#include "tree.h"
#include "symbol.h"



//storing the references here:
//the actual type. here we store the <type> if the node is:
//<var_type>:  id : <type> or
//<declaration>: type id = <type> and
typedef struct type_info
{
	enum
	{
		t_idk,
		t_intK,
		t_boolK,
		t_stringK,
		t_recordK,
		t_arrayK,
		t_nullK,
		t_typeK, //proper type.
	} kind;

	size_t nesting_depth;
	size_t record_nesting_depth; //for nested records
	size_t function_nesting_depth; //for nested functions

	//char* id; //in case it's a type that is an id.
	char* func_id; //used to store the id of the function.
	char* type_id; //used to store the id of the a variable in the collection phase, think of it as a linked_list with id's. This is used when there's id nesting.
	char is_record;
	char has_return;
	char is_a_type;
	char is_a_function;
	char is_a_variable;
	char is_a_record;
	char is_verified;
	char is_record_element; //true if the type is an element of a record.
	char visited; //true if we've visited this node.
	struct type_info* array_type_info;
	struct type_info* type_info_child; //type id = <type>; this will be the type_info of type; or func id ( hpar decl list ) : <type>
	struct type_info* return_type; //the type of the return statement is stored here. this is only used for a return statement, if this is null, then it's not a return stm.
	VAR_DECL_LIST* record_var_decl_list;
	array* function_var_decl_list_types; //list of var types, in a function.
	array* record_var_decl_list_types; //list of var types, in a  record.
	array* act_list_types; //list of decl types.
	array* stm_list_types; //list of decl types.

	SymbolTable* record_scope; //the scope in which if we were a record we would have our children.
	SymbolTable* sym_table; //the scope in which we belong to.
	SYMBOL* symbol;


	size_t act_list_count;

	//maybe the offset van be in the same value
	int par_var_offset; //parameter and local var offset from base pointer
} type_info;

SymbolTable* type_checker(BODY* AST);


type_info* type_make_record(VAR_DECL_LIST* var_decl_list);
type_info* type_make_array(TYPE* type);

type_info* type_check_STRING(STRING* s, SymbolTable* scope);
type_info* type_check_FUNCTION(FUNCTION* f, SymbolTable* scope);
type_info* type_check_HEAD(HEAD* head, SymbolTable* scope);
type_info* type_check_TAIL(TAIL* tail, SymbolTable* scope);
type_info* type_check_TYPE(TYPE* type, SymbolTable* scope);
type_info* type_check_PAR_DECL_LIST(PAR_DECL_LIST* par_decl_list, SymbolTable* scope);
type_info* type_check_VAR_DECL_LIST(VAR_DECL_LIST* var_decl_list, SymbolTable* scope, array* arr);
type_info* type_check_VAR_TYPE(VAR_TYPE* var_type, SymbolTable* scope);
type_info* type_check_BODY(BODY* body, SymbolTable* scope);
type_info* type_check_DECL_LIST(DECL_LIST* decl_list, SymbolTable* scope);
type_info* type_check_DECL(DECL* decl, SymbolTable* scope);
type_info* type_check_STM_LIST(STM_LIST* stm_list, SymbolTable* scope);
type_info* type_check_STM(STM* stm, SymbolTable* scope);
type_info* type_check_VARIABLE(VARIABLE* var, SymbolTable* scope);
type_info* type_check_EXP(EXP* exp, SymbolTable* scope);
type_info* type_check_TERM(TERM* term, SymbolTable* scope);
type_info* type_check_ACT_LIST(ACT_LIST* act_list, SymbolTable* scope, array* arr);
type_info* type_check_EXP_LIST(EXP_LIST * exp_list, SymbolTable* scope, array* arr);

