#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "code_gen.h"
#include "type_checker.h"
#include "falcon_string.h"

char* code_gen_asm_push_all_registers()
{
	//TODO: GHJ: generate this only once and put into a hashmap for faster lookup.
	//return value registers
	char* rax = "pushq %rax\n";
	//general purpose registers
	char* rbx = "pushq %rbx\n";
	char* rcx = "pushq %rcx\n";
	char* rdx = "pushq %rdx\n";

	char* rdi = "pushq %rdi\n";
	char* rsi = "pushq %rsi\n";

	char* r8 = "pushq %r8\n";
	char* r9 = "pushq %r9\n";
	char* r10 = "pushq %r10\n";
	char* r11 = "pushq %r11\n";
	char* r12 = "pushq %r12\n";
	char* r13 = "pushq %r13\n";
	char* r14 = "pushq %r14\n";
	char* r15 = "pushq %r15\n";

	//concatenate them to a single string.
	string_builder* sb = string_builder_new();
	string_builder_append_chars(sb, rax);
	string_builder_append_chars(sb, rbx);
	string_builder_append_chars(sb, rcx);
	string_builder_append_chars(sb, rdx);

	string_builder_append_chars(sb, rdi);
	string_builder_append_chars(sb, rsi);

	string_builder_append_chars(sb, r8);
	string_builder_append_chars(sb, r9);
	string_builder_append_chars(sb, r10);
	string_builder_append_chars(sb, r11);
	string_builder_append_chars(sb, r12);
	string_builder_append_chars(sb, r13);
	string_builder_append_chars(sb, r14);
	string_builder_append_chars(sb, r15);
	char* string = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return string;
}

char* code_gen_asm_pop_all_registers()
{
	//TODO: GHJ: generate this only once and put into a hashmap for faster lookup.
	//return value registers
	char* rax = "popq %rax\n";
	//general purpose registers
	char* rbx = "popq %rbx\n";
	char* rcx = "popq %rcx\n";
	char* rdx = "popq %rdx\n";

	char* rdi = "popq %rdi\n";
	char* rsi = "popq %rsi\n";

	char* r8 = "popq %r8\n";
	char* r9 = "popq %r9\n";
	char* r10 = "popq %r10\n";
	char* r11 = "popq %r11\n";
	char* r12 = "popq %r12\n";
	char* r13 = "popq %r13\n";
	char* r14 = "popq %r14\n";
	char* r15 = "popq %r15\n";

	//concatenate them to a single string.
	string_builder* sb = string_builder_new();
	string_builder_append_chars(sb, rax);
	string_builder_append_chars(sb, rbx);
	string_builder_append_chars(sb, rcx);
	string_builder_append_chars(sb, rdx);

	string_builder_append_chars(sb, rdi);
	string_builder_append_chars(sb, rsi);

	string_builder_append_chars(sb, r8);
	string_builder_append_chars(sb, r9);
	string_builder_append_chars(sb, r10);
	string_builder_append_chars(sb, r11);
	string_builder_append_chars(sb, r12);
	string_builder_append_chars(sb, r13);
	string_builder_append_chars(sb, r14);
	string_builder_append_chars(sb, r15);
	char* string = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return string;
}

char* codegen_asm_push_base_pointer()
{
	return "pushq %rbp\n";
}

char* codegen_asm_move_stack_pointer_to_base_pointer()
{
	return "movq %rsp, %rbp\n";
}

char* codegen_asm_move_base_pointer_to_stack_pointer()
{
	return "movq %rbp, %rsp\n";
}

char* codegen_asm_pop_base_pointer()
{
	return "popq %rbp\n";
}

char* codegen_asm_ret()
{
	return "ret\n";
}

char* start_func_label(char* func_id, int* global_func_label_counter)
{

	string_builder* sb = string_builder_new();

	string_builder_append_chars(sb, func_id);
	string_builder_append_chars(sb, "_");
	string_builder_append_int(sb, global_func_label_counter);
	string_builder_append_chars(sb, ":\n");
	char* result = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);

	*global_func_label_counter += 1;

	return result;

	/*
	//We add 4 to the length because we want to add counter+":\n" to the label
	size_t func_label_length = strlen(func_id) + 5;
	char* func_label_str = Calloc(func_label_length, sizeof(char));

	//converting global_func_label_counter to string
	char count_str[20]; //assuming we dont need more labels than 20 digit numbers can represent
	memset(count_str, 0, sizeof(count_str));
	sprintf(count_str, "%d", *global_func_label_counter);

	char* result = strcat(func_label_str, func_id);
	result = strcat(result, "_");
	result = strcat(result, count_str);
	result = strcat(result, ":\n");

	*global_func_label_counter += 1;

	return result;
	*/
}

char* IR_prologue()
{
	char* prologue_str = "pushq %rbp\nmovq %rsp, %rbp\n";
	return prologue_str;
}

char* push_local_var_decl(int intconst, int kind)
{

	if (intconst)
	{
		//no-op. just to clear gcc warnings. TODO: refactor me.
	}

	/*Push uninitialized variable, thus has value of 0.
	 *
	 * Here we should also insert the varialbe in variable map
	 * and save it with the stack index from base pointer,
	 * so we can find it when we need it again. */
	char* declare = "";
	if (kind == 2) //local var
	{
		//if (!is_in_main)
		declare = "pushq $0   #pushing empty value for later assignment of variable\n";
		//declare = "addq $8, %rsp   #pushing empty value for later assignment of variable\n";
	}
	else if (kind == 3) //local record
	{
		declare = "pushq $0   #pushing empty value for later assignment/pointing of record\n";
	}
	return declare;
}

char* end_func_label(char* func_id, int global_func_lable_counter, int neasting_depth)
{
	int number = (global_func_lable_counter)+neasting_depth - 1;

	string_builder* sb = string_builder_new();
	string_builder_append_chars(sb, "end_");
	string_builder_append_chars(sb, func_id);
	string_builder_append_chars(sb, "_");
	string_builder_append_int(sb, &number);
	string_builder_append_chars(sb, ":\n");
	char* result = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);

	return result;
}


char* ASM_get_end_label(char* func_id)
{
	char* new_string = string_substring_from_right(func_id, 2);

	string_builder* sb = string_builder_new();
	string_builder_append_chars(sb, "end_");
	string_builder_append_chars(sb, new_string);
	string_builder_append_chars(sb, "\n");

	char* result = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return result;
}

char* IR_epilogue()
{
	char* epilogue_str = "movq %rbp, %rsp\npopq %rbp\n";
	return epilogue_str;
}

char* ASM_return()
{
	return "ret\n";
}

char* push_parameters(code_info* data)
{
	//insert parameters into variable map with their name and stack base pointer offset
	char* result = "";
	if (data->type == int_type)
	{
		//push $(length of num);

		//counting the amount of digits in the integer thats converted to string
		int the_number = data->num;

		string_builder* sb = string_builder_new();
		string_builder_append_chars(sb, "pushq $");
		string_builder_append_int(sb, &the_number);
		string_builder_append_chars(sb, "    #push param for called function\n");
		char* string = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		result = string;

	}
	return result;
}

char* pop_parameters(int memoryOffset)
{
	string_builder* sb = string_builder_new();
	string_builder_append_chars(sb, "addq $");
	string_builder_append_int(sb, &memoryOffset);
	string_builder_append_chars(sb, ", %rsp    #discard parameter\n");
	char* string = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);

	char* result = string;
	return result;
}

char* call_function(char* func_id)
{
	string_builder* sb = string_builder_new();
	string_builder_append_chars(sb, "call ");
	string_builder_append_chars(sb, func_id);
	string_builder_append_chars(sb, "\n");
	char* string = string_builder_copy_to_char_array(sb);
	char* result = string;

	return result;
}

char* ASM_return_RAX(char* end_lable, EXP* exp)
{
	string_builder* sb = string_builder_new();

	if (exp->val.term->kind == 1)
	{
//if the exp in return is a function returing in rax, do not pop rax because result already in rax
	}
	else
	{
		string_builder_append_chars(sb, "popq %rax\n"); //#    pop if not return exp is not function
	}

	string_builder_append_chars(sb, "jmp ");
	string_builder_append_chars(sb, end_lable);
	//string_builder_append_chars(sb, "\n");
	char* string = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	char* result = string;

	return result;
}

char* ASM_print(type_info* kind, EXP* exp)
{
	string_builder* sb = string_builder_new();
	if (kind->kind == t_intK)
	{
		//first we pop
		if (exp->val.term->kind == 1)
		{
//if the exp in write is a function returing in rax, movq rax to rbx
			string_builder_append_chars(sb, "movq %rax, %rbx     #moving return value in rax to rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //# passing 1. argument in %rdi
		}

		string_builder_append_chars(sb, "leaq printf_format_int(%rip), %rdi\n"); //# passing 1. argument in %rdi
		//string_builder_append_chars(sb, "lea printf_format(\%rip), \%rdi\n"); //# passing 1. argument in %rdi
		//string_builder_append_chars(sb, "movq $printf_format, \%rdi\n"); //# passing 1. argument in %rdi

		string_builder_append_chars(sb, "movq %rbx, %rsi\n"); //# passing 2. argument in %rsi
	}
	else if (kind->kind == t_boolK)
	{
		if (exp->val.term->kind == 1)
		{
//if the exp in write is a function returing in rax, movq rax to rbx
			string_builder_append_chars(sb, "movq %rax, %rbx     #moving return value in rax to rbx\n");
		}
		else
		{
			string_builder_append_chars(sb, "popq %rbx\n"); //# passing 1. argument in %rdi
		}
		string_builder_append_chars(sb, "leaq printf_format_int(%rip), %rdi\n"); //# passing 1. argument in %rdi

		string_builder_append_chars(sb, "movq %rbx, %rsi\n"); //# passing 2. argument in %rsi
	}
	else if (kind->kind == t_stringK)
	{
		string_builder_append_chars(sb, "leaq printf_format_string(%rip), %rdi\n"); //# passing 1. argument in %rdi
		string_builder_append_chars(sb, "popq %rsi\n"); //# passing 2. argument in %rsi
		/*
		string_builder_append_chars(sb, "lea "); //# passing 2. argument in %rsi
		string_builder_append_chars(sb, *cg_exp->ptr_to_static_string_label); //# passing 2. argument in %rsi
		string_builder_append_chars(sb, "(\%rip), \%rsi\n"); //# passing 2. argument in %rsi
		*/
	}

	string_builder_append_chars(sb, "movq $0, %rax\n"); //# no floating point registers used
	string_builder_append_chars(sb, "call printf\n");

	char* string = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);

	return string;
}


char* ASM_assignment_of_var_with_static_link()
{
	return "";
}

//PUSH and POP safe. RCX and RDX are not safe.
char* ASM_get_variable_and_put_in_rcx(int count, int offset, char is_a_function)
{
//pop the value of the assigned var that should be on top of stack,
//unless it returned form a function thus in rax
	int static_link_offset = 16;
	string_builder* sb = string_builder_new();
	if (is_a_function) //then the variable is in rax
	{
		string_builder_append_chars(sb, "movq %rax, %rcx\n");

		char* result = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		return result;
	}

	if (count == 0)
	{
		string_builder_append_chars(sb, "movq ");
		string_builder_append_int(sb, &offset);
		string_builder_append_chars(sb, "(%rbp), %rcx #assign value to position on stack(variable/parameter)\n");
	}
	else if (count > 0)
	{
		string_builder_append_chars(sb, "movq ");
		string_builder_append_int(sb, &static_link_offset);
		string_builder_append_chars(sb, "(%rbp), %rdx     # ASM_get_variable_and_put_in_rcx start of static link finding address to move value in rcx into rdx\n");
		for (int i = 0; i < (count - 1); i++)
		{
			string_builder_append_chars(sb, "movq ");
			string_builder_append_int(sb, &static_link_offset);
			string_builder_append_chars(sb, "(%rdx), %rdx\n");
		}
		string_builder_append_chars(sb, "movq %rcx, ");
		string_builder_append_int(sb, &offset);
		string_builder_append_chars(sb, "(%rdx)       # ASM_get_variable_and_put_in_rcx end of static link finding address to move value in rcx into rdx\n");
	}
	char* result = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return result;
}

//for assignment of value to variable
char* ASM_assign_variable(int count, int offset, EXP* exp)
{
	int static_link_offset = 16;
	string_builder* sb = string_builder_new();
	if (exp->val.term->kind == 1)
	{
//If the expression is assigning variable from result of function
//,thus get from rax
		string_builder_append_chars(sb, "movq %rax, %rcx\n");
	}
	else
	{
		//Pop value of expression from top of stack to r14
		string_builder_append_chars(sb, "popq %rcx\n");
	}

	if (count == 0)
	{
		string_builder_append_chars(sb, "movq %rcx, ");
		string_builder_append_int(sb, &offset);
		string_builder_append_chars(sb, "(%rbp)      #assign value to position on stack(variable/parameter)\n");
	}
	else if (count > 0)
	{
		string_builder_append_chars(sb, "movq ");
		string_builder_append_int(sb, &static_link_offset);
		string_builder_append_chars(sb, "(%rbp), %rbx     #start of static link finding address to move value in basepointer-offset into rbx\n");
		for (int i = 0; i < (count - 1); i++)
		{
			string_builder_append_chars(sb, "movq ");
			string_builder_append_int(sb, &static_link_offset);
			string_builder_append_chars(sb, "(%rbx), %rbx\n");
		}
		string_builder_append_chars(sb, "movq %rcx, ");
		string_builder_append_int(sb, &offset);
		string_builder_append_chars(sb, "(%rbx)       #end of static link finding address to move value in rcx into rbx\n");
	}
	char* result = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return result;
}


/*
//for assignment of value to variable
char* ASM_assign_variable(int count, int offset, EXP* exp)
{
//pop the value of the assigned var that should be on top of stack,
//unless it returned form a function thus in rax

	int static_link_offset = 16;
	string_builder* sb = string_builder_new();
	if (exp->val.term->kind == 1)
	{
//If the expression is assigning variable from result of function
//,thus get from rax
		string_builder_append_chars(sb, "movq %rax, %r14\n");
	}
	else
	{
		//Pop value of expression from top of stack to r14
		string_builder_append_chars(sb, "popq %r14\n");
	}

	if (count == 0)
	{
		string_builder_append_chars(sb, "movq %r14, ");
		string_builder_append_int(sb, &offset);
		string_builder_append_chars(sb, "(%rbp)      #assign value to position on stack(varaible/parameter)\n");
	}
	else if (count > 0)
	{
		string_builder_append_chars(sb, "movq ");
		string_builder_append_int(sb, &static_link_offset);
		string_builder_append_chars(sb, "(%rbp), %r15     #start of static link finding address to move value in r14 into\n");
		for (int i = 0; i < (count - 1); i++)
		{
			string_builder_append_chars(sb, "movq ");
			string_builder_append_int(sb, &static_link_offset);
			string_builder_append_chars(sb, "(%r15), %r15\n");
		}
		string_builder_append_chars(sb, "movq %r14, ");
		string_builder_append_int(sb, &offset);
		string_builder_append_chars(sb, "(%r15)       #end of static link finding address to move value in r14 into\n");
	}
	char* result = string_builder_copy_to_char_array(sb);
	string_builder_destroy(sb);
	return result;
}
*/


//for using of variables or parameters
char* ASM_push_variable_from_static_link(int memoryOffset, int par_var_offset, int count)
{
	if (memoryOffset)
	{
		//no-op //just to prevent gcc warnings for now. TODO: refactor this behaviour.
	}


	//static link should be at offset +16, after the basepointer +8 and its position +8 = +16
	int static_link_address = 16;
	string_builder* sb = string_builder_new();
	if (count == 0)
	{
		string_builder_append_chars(sb, "pushq ");
		string_builder_append_int(sb, &par_var_offset);
		string_builder_append_chars(sb, "(%rbp)\n");
		char* result = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		return result;
	}
	else if (count > 0)
	{
		string_builder_append_chars(sb, "movq ");
		string_builder_append_int(sb, &static_link_address);
		string_builder_append_chars(sb, "(%rbp), %r15     #start of static link push value\n");
		for (int i = 0; i < (count - 1); i++)
		{
			string_builder_append_chars(sb, "movq ");
			string_builder_append_int(sb, &static_link_address);
			string_builder_append_chars(sb, "(%r15), %r15\n");
		}
		string_builder_append_chars(sb, "pushq ");
		string_builder_append_int(sb, &par_var_offset);
		string_builder_append_chars(sb, "(%r15)     #end of static link push value\n");
		char* result = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);
		return result;
	}

	return "no variable form static linked pushed\n";
}

//for pushing the static link address to called function
char* ASM_push_static_link_address(int count)
{
	int static_link_address = 16;

	if (count == 0)
	{
		char* result = "pushq %rbp     #push address of static link\n";
		return result;
	}
	else if (count > 0)
	{
		//find the variable
		string_builder* sb = string_builder_new();
		string_builder_append_chars(sb, "movq ");
		string_builder_append_int(sb, &static_link_address);
		string_builder_append_chars(sb, "(%rbp), %r15\n");

		//dereference static link till right frame pointer
		for (int i = 0; i < (count - 1); i++)
		{
			string_builder_append_chars(sb, "movq ");
			string_builder_append_int(sb, &static_link_address);
			string_builder_append_chars(sb, "(%r15), %r15\n");
		}

		//push variable to static link
		string_builder_append_chars(sb, "push %r15\n");
		char* string = string_builder_copy_to_char_array(sb);
		string_builder_destroy(sb);

		return string;
	}

	return "no static link address pushed\n";
}