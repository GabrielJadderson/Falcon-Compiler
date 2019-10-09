#include "code_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include "falcon_string.h"
#include "symbol.h"
#include "linked_list.h"
#include "code_gen_util.h"
#include "type_checker.h"
#include "logger.h"
#include "array.h"
#include "memory.h"
#include "file_io.h"

extern char FALCON_DEBUG_CODEGEN;
int label_counter = 0;

//The memery offset 4 = 32bit, 8 = 64bit
int memoryOffset = 8;

//The memory offset of first parameter ( static link +8, thus first parameter should be at +8)
int param_start = 16;

int global_func_lable_counter = 0;
int global_condition_op_counter = 0;
int global_runtime_safety_counter = 0;

int global_temp_counter = 0;

int func_neasting_depth = 0;		  //this decrements
int func_neasting_depth_for_main = 0; //this increments

linked_list *data_section;
linked_list *bss_section;

array *array_of_BSS;
array *array_of_DATA;

char *codegen_build_and_get_DATA_section()
{
	string_builder *sb = string_builder_new();
	string_builder_append_chars(sb, ".data\n");
	string_builder_append_chars(sb, "printf_format_int: .string \"%d\\n\"\n");
	string_builder_append_chars(sb, "printf_format_string: .string \"%s\\n\"\n");
	string_builder_append_chars(sb, "printf_format_nl: .string \"\\n\"\n");
	string_builder_append_chars(sb, ".align 8\n");
	string_builder_append_chars(sb, "herpderp_____: \n.space 16394\n");
	string_builder_append_chars(sb, "heap_pointer: \n.space 16394\n");
	string_builder_append_chars(sb, "heap_next: \n.quad 0\n");

	size_t arr_length = array_of_DATA->length;
	for (size_t i = 0; i < arr_length; ++i) //parse rest of data.
	{
		string_builder_append_chars(sb, (char *)array_at(array_of_DATA, i));
		string_builder_append_chars(sb, "\n");
	}
	char *str = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return str;
}

char *codegen_build_and_get_BSS_section()
{
	string_builder *sb = string_builder_new();
	string_builder_append_chars(sb, "\n.bss\n"); //TODO: MAKE THE ENTIRE CODE_GEN WORK BY WITHOUT THIS NEWLINE IN FRONT OF THE SECTION.
	size_t arr_length = array_of_BSS->length;
	for (size_t i = 0; i < arr_length; ++i) //parse rest of data.
	{
		string_builder_append_chars(sb, (char *)array_at(array_of_BSS, i));
	}
	char *str = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return str;
}

void code_gen(BODY *AST, linked_list *list, SymbolTable *root)
{

	label_counter = 0;
	array_of_BSS = array_new();
	array_of_DATA = array_new();

	data_section = linked_list_new();
	bss_section = linked_list_new();
	linked_list_add_last(&data_section, codegen_build_and_get_DATA_section());
	linked_list_add_last(&bss_section, codegen_build_and_get_BSS_section());

	//not needed
	linked_list *main_list = linked_list_new();

	//for some reason Calloc gives segfault (should be fixed now)
	//code_info* data = (code_info*)Calloc(1, sizeof( code_info));
	code_info *data = calloc(1, sizeof(code_info));
	data->offset_stack = linked_list_new();
	// initialize the offset stack
	// Used to maintain the assignment of offset in declaration.
	linked_list_add_last(&(data->offset_stack), "");
	linked_list_set_offset(&(data->offset_stack), 0);
	//start from -4 because the return addres is the last thing pushed before func call
	data->param_offset = param_start;
	//initialize the mainscope and the current scope
	data->main_scope = root;
	data->scope = data->main_scope;

	//adding start of implicit main func
	linked_list_add_last(&main_list, "");
	linked_list_insert_function(&main_list, "main:\n");
	linked_list_insert_function(&main_list, IR_prologue());

	code_gen_BODY(AST, list, main_list, data);

	//adding end of implicis main func
	linked_list_insert_function(&main_list, IR_epilogue());
	linked_list_insert_function(&main_list, "end_main:\n\n");
	linked_list_concatenation(&list, linked_list_remove_function(&main_list));

	//link the list with the main list after the program has been run
	linked_list_concatenation(&data_section, &bss_section);
	linked_list_add_last(&data_section, ".text\n");
	linked_list_add_last(&data_section, ".globl main\n");
	linked_list_concatenation(&data_section, &list);
	*list = *data_section;

	//don't use the linked_list to output to a file.
	file_write_linked_list("output.s", &list);
}

char *codegen_create_unique_label_with_seed(char *seed)
{
	string_builder *sb = string_builder_new();
	string_builder_append_chars(sb, seed);
	string_builder_append_int(sb, &label_counter);
	char *str_final = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	label_counter++;
	return str_final;
}

char *codegen_create_unique_label()
{
	return codegen_create_unique_label_with_seed("label_");
}

void code_gen_STRING(STRING *s, linked_list *list, linked_list *main_list, code_info *data)
{
	if (list || data)
	{
		//no-op. just to prevent GCC warnings.
	}

	if (FALCON_DEBUG_CODEGEN)
		printf("|-- <string>\n");

	switch (s->kind)
	{
	case stringK:
	{
		code_info *cg = (code_info *)Calloc(1, sizeof(code_info));
		cg->ptr_to_static_string_label = (char **)Calloc(1, sizeof(char **));
		char *label_str = codegen_create_unique_label();
		string_builder *sb = string_builder_new();
		string_builder_append_chars(sb, label_str);
		string_builder_append_chars(sb, ": .string \"");
		string_builder_append_chars(sb, s->str);
		string_builder_append_chars(sb, "\"\n");
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		*cg->ptr_to_static_string_label = label_str;

		//add it to the static data instead.
		linked_list_add_last(&data_section, str);

		string_builder *sb_push_label = string_builder_new();
		//string_builder_append_chars(sb_push_label, "pushq \%rbx\n"); //save rbx first?
		string_builder_append_chars(sb_push_label, "leaq ");
		string_builder_append_chars(sb_push_label, label_str);
		string_builder_append_chars(sb_push_label, "(%rip), %rbx\n");
		string_builder_append_chars(sb_push_label, "pushq %rbx\n");
		char *code = string_builder_copy_to_char_array(sb_push_label);
		string_builder_destroy(sb_push_label);

		linked_list_insert_function(&main_list, code);

		s->cg = cg;
	}
	break;
	case string_concatenationK:
	{
	}
	break;
	}
}

void code_gen_FUNCTION(FUNCTION *f, linked_list *list, linked_list *main_list, code_info *data)
{
	func_neasting_depth = 0;
	func_neasting_depth_for_main++;

	//setting old end lable so return statemnet know which lable to jump to
	char *old_func_lable = data->func_lable;

	//setting scope for
	SymbolTable *old_scope = data->scope; //not used so far
	data->scope = f->scope;

	// initialize the offset stack
	linked_list_add_last(&(data->offset_stack), "");
	linked_list_set_offset(&(data->offset_stack), 0);

	char *start_lable = start_func_label(f->func_head->function_id, &global_func_lable_counter);
	char *prologue = IR_prologue();

	/* insert function lable in symbol table to be found in call instructions*/
	SYMBOL *symbol = getSymbol(f->type->sym_table, f->func_head->function_id);
	symbol->func_label = start_lable;

	//add new linked_list enty to main_list->func, then inserting code for the function entry
	linked_list_add_last(&main_list, "");
	linked_list_insert_function(&main_list, start_lable);
	linked_list_insert_function(&main_list, prologue);

	code_gen_HEAD(f->func_head, list, main_list, data);

	//setting end lable so return statement knows which lable to jump to
	data->func_lable = ASM_get_end_label(start_lable);

	code_gen_BODY(f->func_body, list, main_list, data);

	//setting end lable so return statement knows which lable to jump to may change after body because of neasted functions
	data->func_lable = old_func_lable;

	code_gen_TAIL(f->func_tail, list, main_list, data);

	//concatenation the function to the list
	linked_list_concatenation(&list, linked_list_remove_function(&main_list));

	//unsettig scope for static_link
	data->scope = old_scope;
	//data->scope = data->main_scope;

	func_neasting_depth--;
	func_neasting_depth_for_main--;
}

void code_gen_HEAD(HEAD *head, linked_list *list, linked_list *main_list, code_info *data)
{
	//code_gen_TYPE(head->return_type);

	//set the data kind to be func_param
	data->kind = func_param;

	code_gen_PAR_DECL_LIST(head->params, list, main_list, data);

	//unset the data kind to none
	data->kind = none;
}

void code_gen_TAIL(TAIL *tail, linked_list *list, linked_list *main_list, code_info *data)
{
	if (list || data)
	{
		//no-op. just to prevent GCC warnings.
	}

	//return start pointer to last point after deleting everything behind it
	char *end_lable = end_func_label(tail->end_id, global_func_lable_counter, func_neasting_depth);
	char *epilogue = IR_epilogue();

	//linked_list_add_last(&list, end_lable);
	//linked_list_add_last(&list, epilogue);
	linked_list_insert_function(&main_list, end_lable);
	linked_list_insert_function(&main_list, epilogue);

	//linked_list_add_last(&list, ASM_return());
	linked_list_insert_function(&main_list, ASM_return());
}

void code_gen_TYPE(TYPE *type, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (type->kind)
	{
	case type_idK:
	{
	}
	break;

	case type_intK:
	{
	}
	break;

	case type_boolK:
	{
	}
	break;

	case type_arrayK:
	{
		code_gen_TYPE(type->val.type, list, main_list, data);
	}
	break;

	case type_recordK:
	{
		//we increment the dept here because we dont want var_types within records to be appended to main
		//maybe not relevant because records are saved on the heap
		data->kind = record_var;
		code_gen_VAR_DECL_LIST(type->val.var_decl_list, list, main_list, data);
		data->kind = none;
	}
	break;

	case type_stringK:
	{
	}
	break;

	default:
		break;
	}
}

void code_gen_PAR_DECL_LIST(PAR_DECL_LIST *par_decl_list, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (par_decl_list->kind)
	{
	case par_decl_list_var_decl_listK:
	{
		code_gen_VAR_DECL_LIST(par_decl_list->val.var_decl_list, list, main_list, data);
		//the parameter offset is reset to -4
		//because the first position for the first parameter is before the return address
		data->param_offset = param_start;
	}
	break;
	case par_decl_list_emptyK:
	{
	}
	break;

	default:
		break;
	}
}

void code_gen_VAR_DECL_LIST(VAR_DECL_LIST *var_decl_list, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (var_decl_list->kind)
	{
	case var_decl_list_pairK:
	{
		code_gen_VAR_TYPE(var_decl_list->val.pair.head, list, main_list, data);
		code_gen_VAR_DECL_LIST(var_decl_list->val.pair.tail, list, main_list, data);
	}
	break;

	case var_decl_var_typeK:
	{
		code_gen_VAR_TYPE(var_decl_list->val.var_type, list, main_list, data);
	}
	break;

	default:
		break;
	}
}

void code_gen_VAR_TYPE(VAR_TYPE *var_type, linked_list *list, linked_list *main_list, code_info *data)
{
	if (func_neasting_depth_for_main == 0)
	{
		//insert var to var_map_main
		if (data->kind == local_var)
		{
			char *var_declaration = push_local_var_decl(0, data->kind);
			linked_list_insert_function(&main_list, var_declaration);
			linked_list_set_offset(&(data->offset_stack), memoryOffset);

			//get var_type->id from symbol table at scope of the type/ti, and insert offset into par_var_offset
			//var_type->ti->par_var_offset = linked_list_get_offset(&(data->offset_stack));  //remove this

			SYMBOL *var_symbol = getSymbol(data->scope, var_type->id);
			type_info *ti = var_symbol->value;
			ti->par_var_offset = linked_list_get_offset(&(data->offset_stack));

			//printf("var name: %s, var offset: %d \n", var_symbol->name, ti->par_var_offset);
		}
		else if (data->kind == record_var)
		{
		}
		else
		{
		}
	}
	else
	{
		//insert to variable of parameter offset into symbol table
		if (data->kind == func_param)
		{
			data->param_offset += memoryOffset;

			//get var_type->id from symbol table at scope of type/ti, and insert offset into par_var_offset
			SYMBOL *par_symbol = getSymbol(data->scope, var_type->id);
			type_info *ti = par_symbol->value;
			ti->par_var_offset = data->param_offset;

			//printf("par name: %s, par offset: %d \n", par_symbol->name, ti->par_var_offset);
		}
		else if (data->kind == local_var)
		{

			linked_list_set_offset(&(data->offset_stack), memoryOffset);

			//get var_type->id form symbol table at scope of type/ti, and insert offset into par_var_offset
			//var_type->ti->par_var_offset = linked_list_get_offset(&(data->offset_stack));		//remove this
			SYMBOL *var_symbol = getSymbol(data->scope, var_type->id);
			type_info *ti = var_symbol->value;
			ti->par_var_offset = linked_list_get_offset(&(data->offset_stack));

			//printf("var name: %s, var offset: %d \n", var_symbol->name, ti->par_var_offset);

			char *var_declaration = push_local_var_decl(0, data->kind);
			linked_list_insert_function(&main_list, var_declaration);
		}
	}
	code_gen_TYPE(var_type->type, list, main_list, data);
}

void code_gen_BODY(BODY *body, linked_list *list, linked_list *main_list, code_info *data)
{
	code_gen_DECL_LIST(body->decl_list, list, main_list, data);

	//reset the offset to 0
	linked_list_remove_last(&(data->offset_stack));
	code_gen_STM_LIST(body->stm_list, list, main_list, data);
}

void code_gen_DECL_LIST(DECL_LIST *decl_list, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (decl_list->kind)
	{
	case decl_list_pairK:
	{
		if (data->kind != record_var)
		{
			data->kind = local_var;
		}

		code_gen_DECL(decl_list->pair.head, list, main_list, data);
		code_gen_DECL_LIST(decl_list->pair.tail, list, main_list, data);

		if (data->kind == local_var)
		{
			data->kind = none;
		}
	}
	break;

	case decl_list_emptyK:

		break;

	default:
		break;
	}
}

void code_gen_DECL(DECL *decl, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (decl->kind)
	{
	case decl_assignK:
	{

		if (decl->val.type->kind == type_recordK)
		{
			data->kind = record_var;
		}
		else
		{
			data->kind = local_var;
		}

		if (func_neasting_depth_for_main == 0)
		{
			//insert var to offset into symbol table and make room for the varialbe in stack

			if (data->kind == local_var)
			{
				linked_list_set_offset(&(data->offset_stack), memoryOffset);

				//get decl->decl_id from symbol table and insert offset into par_var_offset
				SYMBOL *var_symbol = getSymbol(data->scope, decl->decl_id);
				type_info *ti = var_symbol->value;
				ti->par_var_offset = linked_list_get_offset(&(data->offset_stack));
				//printf("var name: %s, var offset: %d \n", decl->decl_id, decl->type->par_var_offset);

				char *var_declaration = push_local_var_decl(0, data->kind);
				linked_list_insert_function(&main_list, var_declaration);
			}
			else if (data->kind == record_var)
			{
				char *var_declaration = push_local_var_decl(0, data->kind);
				linked_list_insert_function(&main_list, var_declaration);
			}
		}
		else
		{
			//insert var to offset into symbol table and make room for the varialbe in stack

			if (data->kind == local_var)
			{

				linked_list_set_offset(&(data->offset_stack), memoryOffset);

				//get decl->decl_if from symbol table and insert offset into par_var_offset
				SYMBOL *var_symbol = getSymbol(data->scope, decl->decl_id);
				type_info *ti = var_symbol->value;
				ti->par_var_offset = linked_list_get_offset(&(data->offset_stack));
				//printf("var name: %s, var offset: %d \n", decl->decl_id, decl->type->par_var_offset);

				//maybe should differentiate between var and record as above ^
				char *var_declaration = push_local_var_decl(0, data->kind);
				linked_list_insert_function(&main_list, var_declaration);
			}
		}

		//data->kind == local_var;

		code_gen_TYPE(decl->val.type, list, main_list, data);
	}
	break;

	case decl_functionK:
	{
		code_gen_FUNCTION(decl->val.function, list, main_list, data);
	}
	break;

	case decl_varsK:
	{
		if (decl->val.type->kind == type_recordK)
		{
			data->kind = record_var;
		}
		else
		{
			data->kind = local_var;
		}

		code_gen_VAR_DECL_LIST(decl->val.var_decl_list, list, main_list, data);

		data->kind = local_var;
	}
	break;

	default:
		break;
	}
}

void code_gen_STM_LIST(STM_LIST *stm_list, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (stm_list->kind)
	{
	case stm_list_pairK:
	{
		code_gen_STM(stm_list->val.pair.head, list, main_list, data);
		code_gen_STM_LIST(stm_list->val.pair.tail, list, main_list, data);
	}
	break;

	case stm_list_statementK:
	{
		code_gen_STM(stm_list->val.stm, list, main_list, data);
	}
	break;

	default:
		break;
	}
}

void code_gen_STM(STM *stm, linked_list *list, linked_list *main_list, code_info *data)
{
	if (FALCON_DEBUG_CODEGEN)
		printf("|-- <stm> %d\n", stm->kind);
	switch (stm->kind)
	{
	case stm_returnK:
	{
		code_gen_EXP(stm->val.exp, list, main_list, data);

		type_info *ti_exp = stm->val.exp->type;

		string_builder *sb = string_builder_new();

		//id case.
		if (ti_exp->kind == t_idk)
		{
		}
		else if (ti_exp->kind == t_intK || ti_exp->kind == t_boolK)
		{
			string_builder_append_chars(sb, ASM_return_RAX(data->func_lable, stm->val.exp));
		}
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case stm_writeK:
	{
		//Write assembly code for the expression
		//get back type and in what register it was placed in
		//reference it back in printf.
		code_gen_EXP(stm->val.exp, list, main_list, data);
		//printf("write exp kind: %d, term kind %d\n", stm->val.exp->kind, stm->val.exp->val.term->kind);
		type_info *ti_exp = stm->val.exp->type;
		//cg_data* cg_exp = stm->val.exp->cg_data; //not used

		char *string = ASM_print(ti_exp, stm->val.exp);

		linked_list_insert_function(&main_list, string);
	}
	break;

	case stm_allocateK: //https://imada.sdu.dk/~kslarsen/dm546/Material/a4.pdf
	{
		//used only for records.
		code_gen_VARIABLE(stm->val.var, list, main_list, data);
	}
	break;

	case stm_allocate_lengthK:
	{

		/**  we don't have out of mem checks. TODO: implement mem checks.
		 * code for <expression>
		 * (code for out-of-memory check) 
		 * mov “heap-counter”, “address of <id-expression>”
		 * add “<expression>-result”, “heap-counter”
		 */

		code_gen_EXP(stm->val.allocate_length.exp, list, main_list, data);

		string_builder *sb = string_builder_new();
		//TODO: CHECK IF OUT OF MEMORY.

		string_builder_append_chars(sb, "popq %rcx\n"); //pop exp into rcx (length)

		data->stm_kind = assignment_stm;
		code_gen_VARIABLE(stm->val.allocate_length.var, list, main_list, data);
		char *push_variable_base_pointer = ASM_push_static_link_address(data->static_link_count);
		data->stm_kind = none_stm;

		string_builder_append_chars(sb, push_variable_base_pointer); //push the base pointer of the var
		string_builder_append_chars(sb, "popq %rbx\n");				 //pop it into rbx

		//move value of heap_next into
		string_builder_append_chars(sb, "movq $heap_next, %r14\n"); //assign variable to rax
		string_builder_append_chars(sb, "movq (%r14), %r14\n");		//assign variable to rax

		//-24(rbp)
		//assign heap_next = heap_next + exp;
		//movq	heap_next(%rip), %rax
		//addq	%rcx, (%rax)

		//assign the first element in the heap to be equal to the length.
		string_builder_append_chars(sb, "movq $heap_pointer, %r15\n");
		string_builder_append_chars(sb, "addq %r14, %r15\n"); //go to a specific element
		//string_builder_append_chars(sb, "movq (%r15), %r15\n"); //deref
		string_builder_append_chars(sb, "movq %rcx, (%r15)\n"); //add exp
		string_builder_append_chars(sb, "addq $8, %r15\n");		//add 1
		//string_builder_append_chars(sb, "movq $0xabcd, (%r15)\n");
		string_builder_append_chars(sb, "movq %r15, ");
		string_builder_append_int(sb, &data->static_link_var_offset); //add offset so -32(%rbx) where -32 is offset
		string_builder_append_chars(sb, "(%rbx)\n");				  //use the address

		//lastly update heap counter
		string_builder_append_chars(sb, "movq $heap_next, %r14\n"); //assign variable to rax
		string_builder_append_chars(sb, "movq (%r14), %r14\n");		//assign variable to rax
		//string_builder_append_chars(sb, "addq %rcx, %r14\n");
		string_builder_append_chars(sb, "inc %rcx\n");
		string_builder_append_chars(sb, "movq %rcx, %rax\n");
		string_builder_append_chars(sb, "movq $8, %r13\n");
		string_builder_append_chars(sb, "mulq %r13\n");
		string_builder_append_chars(sb, "addq %rax, %r14\n");
		//string_builder_append_chars(sb, "addq $8, %r14\n");
		string_builder_append_chars(sb, "movq $heap_next, %r15\n");
		string_builder_append_chars(sb, "movq %r14, (%r15)\n");

		//h: .space 1000000
		//heap_next: .space 8
		//movq $h, %rbx
		//movq (%rbx), r8 //her mover vi hvad der er inde i (%rbx) første element
		//addq $8, %rbx /her adder vi 8 = 1 i 64bit til rbx så vi går ned i array'en.
		//movq %rbx, (%rbx)

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case stm_assignmentK:
	{
		/*
		code_gen_EXP(stm->val.assignment.exp, list, main_list, data);

		data->stm_kind = assignment_stm;

		data->variable_is_assign = 1;
		code_gen_VARIABLE(stm->val.assignment.var, list, main_list, data);
		data->variable_is_assign = 0;
		data->stm_kind = none_stm;

		string_builder *sb = string_builder_new();

		if ((data->variable_is_array_access))
		{
			string_builder_append_chars(sb, "#test\n");
			char *res = ASM_assign_variable(data->static_link_count, data->static_link_var_offset, stm->val.exp);
			string_builder_append_chars(sb, res);
		}
		else
		{
			string_builder_append_chars(sb, "#weasel\n");
			string_builder_append_chars(sb, "popq %rdx\n"); //variable
			string_builder_append_chars(sb, "popq %rax\n"); //exp
			string_builder_append_chars(sb, "movq %rax, (%rdx)\n");
		}
		char *string = string_builder_copy_to_char_array(sb);
		string_builder_append_chars(sb, string);
		string_builder_destroy(sb);
		//print string to code list
		linked_list_insert_function(&main_list, string);
	*/

		code_gen_EXP(stm->val.assignment.exp, list, main_list, data);

		data->stm_kind = assignment_stm;

		code_gen_VARIABLE(stm->val.assignment.var, list, main_list, data);
		char *string = ASM_assign_variable(data->static_link_count, data->static_link_var_offset, stm->val.exp);
		//print string to code list
		linked_list_insert_function(&main_list, string);

		data->stm_kind = none_stm;
	}

	break;

	case stm_ifK: //https://en.wikibooks.org/wiki/X86_Assembly/Control_Flow
	{
		/*
			* code for <expression>
			* cmp "<expression>-result", "true"
			* jne end_if #assuming equality
			* code for <stm1>
			* jmp end_if
			* end_if:
			*/
		//TODO: implement it how kim wants it. exactly as the way he wants it,
		//by comparing in exp and putting a value for true or false in expression itself and pushing either true or false in expression.
		//and popping and comparing that value in here.

		//look into lazy AND and OR expressions..
		code_gen_EXP(stm->val.if_stm.exp, list, main_list, data);

		char *if_end_label = codegen_create_unique_label_with_seed("end_if_");

		string_builder *sb = string_builder_new();

		string_builder_append_chars(sb, "popq %rax\n");		  //cmp
		string_builder_append_chars(sb, "movq $1, %rbx\n");   //cmp
		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp
		string_builder_append_chars(sb, "jne ");			  //cmp
		string_builder_append_chars(sb, if_end_label);		  //add end_if label
		string_builder_append_chars(sb, "\n");
		char *if_first_part = string_builder_copy_to_char_array(sb);
		string_builder_clear(sb);

		linked_list_insert_function(&main_list, if_first_part);

		code_gen_STM(stm->val.if_stm.stm, list, main_list, data);

		//jmp end_if
		string_builder_append_chars(sb, "jmp ");
		string_builder_append_chars(sb, if_end_label);
		string_builder_append_chars(sb, "\n");
		//end_if:
		string_builder_append_chars(sb, if_end_label);
		string_builder_append_chars(sb, ":\n");
		char *if_second_part = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, if_second_part);
	}
	break;

	case stm_if_elseK:
	{
		/*
			 * code for <expression>
			 * cmp "<expression>-result", "true"
			 * jne else_part
			 * code for <stm1>
			 * jmp end_if
			 * else_part:
			 * code for <stm_2>
			 * end_if:
			 */

		code_gen_EXP(stm->val.if_else_stm.exp, list, main_list, data);

		char *else_part_label = codegen_create_unique_label_with_seed("else_part_");

		string_builder *sb = string_builder_new();

		string_builder_append_chars(sb, "popq %rax\n");		  //cmp
		string_builder_append_chars(sb, "movq $1, %rbx\n");   //cmp
		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp
		string_builder_append_chars(sb, "jne ");			  //cmp
		string_builder_append_chars(sb, else_part_label);	 //add end_if label
		string_builder_append_chars(sb, "\n");
		char *if_else_first_part = string_builder_copy_to_char_array(sb);
		string_builder_clear(sb);

		linked_list_insert_function(&main_list, if_else_first_part);

		//---
		code_gen_STM(stm->val.if_else_stm.stm_then, list, main_list, data);

		char *if_else_end = codegen_create_unique_label_with_seed("end_if_else_");

		//jmp end_if
		string_builder_append_chars(sb, "jmp ");
		string_builder_append_chars(sb, if_else_end);
		string_builder_append_chars(sb, "\n");
		//end_if:
		string_builder_append_chars(sb, else_part_label);
		string_builder_append_chars(sb, ":\n");
		char *if_second_part = string_builder_copy_to_char_array(sb);
		string_builder_clear(sb);

		linked_list_insert_function(&main_list, if_second_part);

		code_gen_STM(stm->val.if_else_stm.stm_else, list, main_list, data);

		string_builder_append_chars(sb, if_else_end);
		string_builder_append_chars(sb, ":\n");
		char *if_third_part = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, if_third_part);
	}
	break;

	case stm_whileK:
	{

		/* while_start:
			 * code for <expression>
			 * cmp "<expression>-result", "true"
			 * jne while_end
			 * code for <stm>
			 * while_end:
			 */

		char *while_start_label = codegen_create_unique_label_with_seed("while_start_");
		string_builder *sb = string_builder_new();
		string_builder_append_chars(sb, while_start_label);
		string_builder_append_chars(sb, ":\n");
		char *while_start_label_code = string_builder_copy_to_char_array(sb);

		linked_list_insert_function(&main_list, while_start_label_code);

		string_builder_clear(sb);

		code_gen_EXP(stm->val.while_stm.exp, list, main_list, data);

		char *while_end_label = codegen_create_unique_label_with_seed("while_end_");

		string_builder_append_chars(sb, "popq %rax\n");		  //cmp
		string_builder_append_chars(sb, "movq $1, %rbx\n");   //cmp
		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp
		string_builder_append_chars(sb, "jne ");			  //cmp
		string_builder_append_chars(sb, while_end_label);	 //add end_if label
		string_builder_append_chars(sb, "\n");

		char *if_else_first_part = string_builder_copy_to_char_array(sb);
		string_builder_clear(sb);

		linked_list_insert_function(&main_list, if_else_first_part);

		code_gen_STM(stm->val.while_stm.stm, list, main_list, data);

		//end_while:
		string_builder_append_chars(sb, "jmp ");
		string_builder_append_chars(sb, while_start_label);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, while_end_label);
		string_builder_append_chars(sb, ":\n");
		char *while_end_code = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, while_end_code);
	}
	break;

	case stm_listK:
	{
		code_gen_STM_LIST(stm->val.list, list, main_list, data);
	}
	break;

	default:
		break;
	}
}

void code_gen_VARIABLE(VARIABLE *var, linked_list *list, linked_list *main_list, code_info *data)
{
	data->variable_is_array_access = 0;
	switch (var->kind)
	{
	case variable_idK:
	{
		//printf("start dump\n");
		//dumpSymbolTable(data->scope);
		//printf("end dump \n");

		int count = 0;
		SYMBOL *var_symbol = getSymbolCount(data->scope, var->val.id, &count);
		type_info *ti = var_symbol->value;
		int par_var_offset = ti->par_var_offset;
		//printf("Par_var name: %s , symbol par_var_offset: %d, scope count:%d \n", var_symbol->name, par_var_offset, count);
		//printf("var id: %s, var offset: %d, scope count: %d \n", var->val.id, var->type->par_var_offset, count);

		if (data->stm_kind == assignment_stm)
		{
			//save the variable offset and scope count for insertion of data into the position in stack, by use of static link

			//printf("Assignment:\n var id: %s, var offset: %d, scope count: %d \n", var->val.id, var->type->par_var_offset, count);
			data->static_link_count = count;
			data->static_link_var_offset = par_var_offset;
		}
		else
		{
			//push varialbe to stack for use in the expression of a statement

			//printf("var id: %s, var offset: %d, scope count: %d \n", var->val.id, var->type->par_var_offset, count);
			string_builder *sb = string_builder_new();
			string_builder_append_chars(sb, ASM_push_variable_from_static_link(memoryOffset, par_var_offset, count));
			char *str = string_builder_copy_to_char_array(sb);
			string_builder_destroy(sb);

			linked_list_insert_function(&main_list, str);
		}
	}
	break;

	case variable_arrayAccessK:
	{
		data->variable_is_array_access = 1;
		code_gen_VARIABLE(var->val.array_access.var, list, main_list, data);
		code_gen_EXP(var->val.array_access.exp, list, main_list, data);
		string_builder *sb = string_builder_new();
		//string_builder_append_chars(sb, "#HELP!\n");

		string_builder_append_chars(sb, "popq %r15\n");
		string_builder_append_chars(sb, "popq %r14\n");
		string_builder_append_chars(sb, "movq $8, %r13\n"); //set offset.
		string_builder_append_chars(sb, "movq %r15, %rax\n");
		string_builder_append_chars(sb, "mulq %r13\n");
		string_builder_append_chars(sb, "addq %rax, %r14\n");

		if (data->variable_is_assign)
		{
			string_builder_append_chars(sb, "pushq %r14\n");
		}
		else
		{
			string_builder_append_chars(sb, "pushq (%r14)\n");
		}

		//string_builder_append_chars(sb, "pushq %r14\n");
		//string_builder_append_chars(sb, "pushq (%r14)\n");
		//string_builder_append_chars(sb, "movq $heap_pointer, %rbx\n");
		//string_builder_append_chars(sb, "addq %r14, %rbx\n");
		//string_builder_append_chars(sb, "pushq %(rbx)\n");
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		linked_list_insert_function(&main_list, str);
	}

	break;

	case variable_access_idK:
	{
		code_gen_VARIABLE(var->val.access.var, list, main_list, data);
	}

	break;

	default:
		break;
	}
}

void code_gen_EXP(EXP *exp, linked_list *list, linked_list *main_list, code_info *data)
{
	switch (exp->kind)
	{
	case exp_timesK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "imulq %rbx, %rax\n"); //multiply into rax
		string_builder_append_chars(sb, "pushq %rax\n");	   //push the result.
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_divK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the left.
		}

		/***** Run-time check for division by zero *****/

		//saving values to stack
		string_builder_append_chars(sb, "pushq %rax\npushq %rbx\n");

		string_builder_append_chars(sb, "cmp $0, %rax\n");

		string_builder_append_chars(sb, "je div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "jmp end_div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, ":\n");

		string_builder_append_chars(sb, "movq stderr(%rip), %rax\nmovq %rax, %rcx\nmovl $13, %edx\nmovl $1, %esi\nmovl $3, %edi\ncall fwrite\n");

		string_builder_append_chars(sb, "end_div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, ":\n");

		//poping saved values from stack
		string_builder_append_chars(sb, "popq %rbx\npopq %rax\n");

		global_runtime_safety_counter++;

		//saving values to stack
		string_builder_append_chars(sb, "pushq %rax\npushq %rbx\n");

		string_builder_append_chars(sb, "cmp $0, %rbx\n");

		string_builder_append_chars(sb, "je div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "jmp end_div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, ":\n");

		string_builder_append_chars(sb, "movq stderr(%rip), %rax\nmovq %rax, %rcx\nmovl $13, %edx\nmovl $1, %esi\nmovl $3, %edi\ncall fwrite\n");

		string_builder_append_chars(sb, "end_div_by_zero");
		string_builder_append_int(sb, &global_runtime_safety_counter);
		string_builder_append_chars(sb, ":\n");

		//poping saved values from stack
		string_builder_append_chars(sb, "popq %rbx\npopq %rax\n");

		global_runtime_safety_counter++;
		/*****End of run-time check for divition by zerio*****/

		string_builder_append_chars(sb, "cqto\nidivq %rbx\n"); //divide into rax this is integer division
		string_builder_append_chars(sb, "pushq %rax\n");	   //push the result.
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_plusK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "addq %rbx, %rax\n"); //add into rax
		string_builder_append_chars(sb, "pushq %rax\n");	  //push the result.
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_minusK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.

		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %r11\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the left.
		}

		string_builder_append_chars(sb, "subq %rbx, %r11\n");						  //substract the two.
		string_builder_append_chars(sb, "pushq %r11       #end of sub expression\n"); //push the result.
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_equalityK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "cmpq %rbx, %rax\n"); //cmp the two.
		string_builder_append_chars(sb, "je TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "pushq $0\n");
		string_builder_append_chars(sb, "jmp END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");
		string_builder_append_chars(sb, "pushq $1\n");
		string_builder_append_chars(sb, "END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		global_condition_op_counter++;

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_non_equalityK:
	{

		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "cmpq %rbx, %rax\n"); //cmp the two.
		string_builder_append_chars(sb, "jne TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "pushq $0\n");
		string_builder_append_chars(sb, "jmp END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");
		string_builder_append_chars(sb, "pushq $1\n");
		string_builder_append_chars(sb, "END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		global_condition_op_counter++;

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_greaterK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp the two.
		string_builder_append_chars(sb, "jg TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "pushq $0\n");
		string_builder_append_chars(sb, "jmp END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");
		string_builder_append_chars(sb, "pushq $1\n");
		string_builder_append_chars(sb, "END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		global_condition_op_counter++;

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_lesserK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp the two.
		string_builder_append_chars(sb, "jl TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "pushq $0\n");
		string_builder_append_chars(sb, "jmp END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");
		string_builder_append_chars(sb, "pushq $1\n");
		string_builder_append_chars(sb, "END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		global_condition_op_counter++;

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_greater_equalK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp the two.
		string_builder_append_chars(sb, "jge TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "pushq $0\n");
		string_builder_append_chars(sb, "jmp END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");
		string_builder_append_chars(sb, "pushq $1\n");
		string_builder_append_chars(sb, "END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		global_condition_op_counter++;

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_lesser_equalK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "cmpq %rax, %rbx\n"); //cmp the two.
		string_builder_append_chars(sb, "jle TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "pushq $0\n");
		string_builder_append_chars(sb, "jmp END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, "\n");

		string_builder_append_chars(sb, "TRUE_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");
		string_builder_append_chars(sb, "pushq $1\n");
		string_builder_append_chars(sb, "END_");
		string_builder_append_int(sb, &global_condition_op_counter);
		string_builder_append_chars(sb, ":\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		global_condition_op_counter++;

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_andK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "andq %rbx, %rax\n"); //multiply into rax
		string_builder_append_chars(sb, "pushq %rax\n");	  //push the result.
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_orK:
	{
		//left pushed first, right pushed second.
		//left = rbx, right = rax, we chose rax in the right node because we want the result in rax.
		string_builder *sb = string_builder_new();

		code_gen_EXP(exp->val.op.right, list, main_list, data);
		if (exp->val.op.right->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
			string_builder_append_chars(sb, "movq %rax, %rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //pop the right.
		}

		code_gen_EXP(exp->val.op.left, list, main_list, data);
		if (exp->val.op.left->val.term->kind == 1)
		{
			//if the exp is a function returing in rax
		}
		else
		{
			string_builder_append_chars(sb, "popq %rax\n"); //pop the left.
		}

		string_builder_append_chars(sb, "orq %rbx, %rax\n"); //multiply into rax
		string_builder_append_chars(sb, "pushq %rax\n");	 //push the result.
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case exp_termK:
	{
		code_gen_TERM(exp->val.term, list, main_list, data);
		exp->cg = exp->val.term->cg;
	}
	break;

	default:
		break;
	}
}

void code_gen_TERM(TERM *term, linked_list *list, linked_list *main_list, code_info *data)
{
	if (FALCON_DEBUG_CODEGEN)
		printf("|-- <term> %d\n", term->kind);

	switch (term->kind)
	{
	case term_varK:
	{
		code_gen_VARIABLE(term->val.var, list, main_list, data);
	}
	break;

	case term_id_act_listK:
	{
		//this is only used for functions <id>(<exp>)
		//setting the kind of exp to be parameters so we know how to handle them in relation to the stack
		data->kind = func_param;
		//printf("term_id_act_listk\n");

		code_gen_ACT_LIST(term->val.id_act_list.act_list, list, main_list, data);
		char *func_lable = getSymbol(term->type->sym_table, term->val.id_act_list.id)->func_label;
		char new_func_lable[strlen(func_lable)];
		sprintf(new_func_lable, "%s", func_lable);
		memset(new_func_lable + strlen(new_func_lable) - 2, '\0', sizeof(char)); //remove colon from lable

		//find basepointer address and push is before return
		int count = 0;
		SYMBOL *var_symbol = getSymbolCount(data->scope, term->val.id_act_list.id, &count);

		//printf("this is the count: %d\n", count);
		char *push_static_link = ASM_push_static_link_address(count);

		linked_list_insert_function(&main_list, push_static_link);
		linked_list_insert_function(&main_list, call_function(new_func_lable));

		//pop parameters after function has been called
		type_info *type_info_function = (type_info *)var_symbol->value;

		size_t parameter_count = 0;
		if (type_info_function->function_var_decl_list_types)
		{
			parameter_count = type_info_function->function_var_decl_list_types->length;
		}

		for (size_t i = 0; i < parameter_count; ++i)
		{
			char *pop_static_link = pop_parameters(memoryOffset); //removing static link from stack
			linked_list_insert_function(&main_list, pop_static_link);
		}

		char *pop_static_link = pop_parameters(memoryOffset); //removing static link from stack
		linked_list_insert_function(&main_list, pop_static_link);

		data->kind = none;
	}
	break;

	case term_parenthesis_expressionK:
	{
		code_gen_EXP(term->val.exp, list, main_list, data);
		term->cg = term->val.exp->cg;
	}
	break;

	case term_negationK:
	{
		code_gen_TERM(term->val.term, list, main_list, data);

		string_builder *sb = string_builder_new();
		string_builder_append_chars(sb, "pop %r14\n");
		string_builder_append_chars(sb, "negq %r14\n");
		string_builder_append_chars(sb, "pushq %r14\n");

		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case term_absoluteK:
	{
		code_gen_EXP(term->val.exp, list, main_list, data);
		term->cg = term->val.exp->cg;

		string_builder *sb = string_builder_new();

		string_builder_append_chars(sb, "popq %rax\n"); //index is stored in rax

		//RAX HAS DA ARRAY POINTER

		/*
		string_builder_append_chars(sb, "movq $heap_pointer, %rbx\n"); //
		string_builder_append_chars(sb, "subq $1, %rbx\n");			   //increment by index.
		string_builder_append_chars(sb, "addq %rax, %rbx\n");		   //increment by index.
		string_builder_append_chars(sb, "pushq (%rbx)\n");			   //increment by index.
*/
		string_builder_append_chars(sb, "subq $8, %rax\n");
		string_builder_append_chars(sb, "movq (%rax), %r14\n"); //increment by index.
		string_builder_append_chars(sb, "pushq %r14\n");

		//h: .space 1000000
		//heap_next: .space 8
		//movq $h, %rbx
		//movq (%rbx), r8 //her mover vi hvad der er inde i (%rbx) første element
		//addq $8, %rbx /her adder vi 8 = 1 i 64bit til rbx så vi går ned i array'en.
		//movq %rbx, (%rbx)

		char *result = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, result);
	}
	break;

	case term_numK:
	{
		code_info *cg = Calloc(1, sizeof(code_info));
		term->cg = cg;

		//printf("val: %d\n",term->val.num);

		data->num = term->val.num;
		data->type = int_type;
		if (data->kind == func_param)
		{

			char *push_param = push_parameters(data);
			linked_list_insert_function(&main_list, push_param);
		}
		else
		{
			string_builder *sb = string_builder_new();
			string_builder_append_chars(sb, "pushq $");
			string_builder_append_int(sb, &term->val.num);
			string_builder_append_chars(sb, "\n");
			char *str = string_builder_copy_to_char_array(sb);
			string_builder_destroy(sb);

			linked_list_insert_function(&main_list, str);
		}

		//t_intK;
		data->type = none_type;
	}
	break;

	case term_trueK: //boolean
	{
		code_info *cg = Calloc(1, sizeof(code_info));
		term->cg = cg;
		string_builder *sb = string_builder_new();
		string_builder_append_chars(sb, "pushq $1\n");
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case term_falseK: //boolean
	{
		code_info *cg = Calloc(1, sizeof(code_info));
		term->cg = cg;

		string_builder *sb = string_builder_new();
		string_builder_append_chars(sb, "pushq $0\n");
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case term_nullK: //null type
	{
		//GHJ: push null = 0 as in c.
		code_info *cg = Calloc(1, sizeof(code_info));
		term->cg = cg;

		string_builder *sb = string_builder_new();
		string_builder_append_chars(sb, "pushq $0\n");
		char *str = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		//linked_list_add_last(&code_list, str);

		linked_list_insert_function(&main_list, str);
	}
	break;

	case term_stringK:
	{
		code_gen_STRING(term->val.string, list, main_list, data);
		term->cg = term->val.string->cg;
	}
	break;

	default:
		break;
	}
}

void code_gen_ACT_LIST(ACT_LIST *act_list, linked_list *list, linked_list *main_list, code_info *data)
{

	if (FALCON_DEBUG_CODEGEN)
		printf("|-- <act_list> %d\n", act_list->kind);

	switch (act_list->kind)
	{
	case act_list_exp_listK:
		code_gen_EXP_LIST(act_list->exp_list, list, main_list, data);
		break;

	case act_list_emptyK:
		break;

	default:
		break;
	}
}

void code_gen_EXP_LIST(EXP_LIST *exp_list, linked_list *list, linked_list *main_list, code_info *data)
{

	if (FALCON_DEBUG_CODEGEN)
		printf("|-- <exp_list> %d\n", exp_list->kind);

	switch (exp_list->kind)
	{
	case exp_list_pairK:
		if (data->kind == func_param)
		{
			code_gen_EXP_LIST(exp_list->val.pair.tail, list, main_list, data);
			code_gen_EXP(exp_list->val.pair.head, list, main_list, data);
		}
		else
		{
			code_gen_EXP(exp_list->val.pair.head, list, main_list, data);
			code_gen_EXP_LIST(exp_list->val.pair.tail, list, main_list, data);
		}

		break;

	case exp_list_expressionK:
		code_gen_EXP(exp_list->val.exp, list, main_list, data);
		break;
	default:
		break;
	}
}

