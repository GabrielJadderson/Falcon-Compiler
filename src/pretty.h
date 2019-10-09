#include "tree.h"

void prettyAST(BODY* AST);
void prettySTRING(STRING* s);
void prettyFUNCTION(FUNCTION* f);
void prettyHEAD(HEAD* head);
void prettyTAIL(TAIL* tail);
void prettyTYPE(TYPE* type);
void prettyPAR_DECL_LIST(PAR_DECL_LIST* par_decl_list);
void prettyVAR_DECL_LIST(VAR_DECL_LIST* var_decl_list);
void prettyVAR_TYPE(VAR_TYPE* var_type);
void prettyBODY(BODY* body);
void prettyDECL_LIST(DECL_LIST* decl_list);
void prettyDECL(DECL* decl);
void prettySTM_LIST(STM_LIST* stm_list);
void prettySTM(STM* stm);
void prettyVARIABLE(VARIABLE* var);
void prettyEXP(EXP* exp);
void prettyTERM(TERM* term);
void prettyACT_LIST(ACT_LIST* act_list);
void prettyEXP_LIST(EXP_LIST * exp_list);
