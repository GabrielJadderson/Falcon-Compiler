#pragma once
#include "stack.h"

void weed_AST(BODY* body);

void weed_FUNCTION(FUNCTION* f);
void weed_HEAD(HEAD* head);
void weed_TAIL(TAIL* tail, FUNCTION* f);


void weed_TYPE(TYPE* type);
void weed_PAR_DECL_LIST(PAR_DECL_LIST* par_decl_list);
void weed_VAR_DECL_LIST(VAR_DECL_LIST* var_decl_list);
void weed_VAR_TYPE(VAR_TYPE* var_type);


void weed_BODY(BODY* body);


void weed_DECL_LIST(DECL_LIST* decl_list);
void weed_DECL(DECL* decl);


void weed_STM_LIST(STM_LIST* stm_list);
void weed_STM(STM* stm);


void weed_VARIABLE(VARIABLE* var);
void weed_EXP(EXP* exp);
void weed_TERM(TERM* term);
void weed_ACT_LIST(ACT_LIST* act_list);
void weed_EXP_LIST(EXP_LIST * exp_list);
