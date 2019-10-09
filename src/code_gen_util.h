#pragma once

#include <string.h>
#include "code_gen.h"
#include "type_checker.h"
#include "tree.h"


char* code_gen_asm_push_all_registers();
char* code_gen_asm_pop_all_registers();
char* code_gen_asm_push_base_pointer();
char* code_gen_asm_move_stack_pointer_to_base_pointer();
char* code_gen_asm_move_base_pointer_to_stack_pointer();
char* code_gen_asm_pop_base_pointer();
char* code_gen_asm_ret();
char* start_func_label(char* func_id, int* global_func_label_counter);

char* end_func_label(char* func_id, int global_func_lable_counter, int neasting_depth);

//get the end lable from a start lable
char* ASM_get_end_label(char* func_id);

char* IR_prologue();
char* push_caller_reg();
char* push_local_var_decl(int intconst, int kind);
/* we dont initialize variables in falcon */
char* ASM_assign_variable(int count, int offset, EXP* exp);
char* IR_epilogue();
char* push_parameters(code_info* data);
char* pop_parameters(int memoryOffset);
char* call_function(char* func_id);
char* ASM_return_RAX(char* end_lable, EXP* exp);
char* ASM_return();
char* ASM_print(type_info* kind, EXP* exp);
char* ASM_assignment_of_var_with_static_link();
char* ASM_push_variable_from_static_link(int memoryOffset, int offset, int count);
char* ASM_push_static_link_address(int count);

char* ASM_get_variable_and_put_in_rcx(int count, int offset, char is_a_function);