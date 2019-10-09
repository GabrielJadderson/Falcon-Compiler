#pragma once

#define HashSize 317

/* SYMBOL will be extended later.
   Function calls will take more parameters later.
*/

typedef struct SYMBOL
{
	char* func_label;
	char *name;
	void* value;
	struct SYMBOL *next;
} SYMBOL;

typedef struct SymbolTable
{
	SYMBOL *table[HashSize];
	struct SymbolTable *next;
} SymbolTable;

unsigned int Hash(char *str);

SymbolTable* initSymbolTable();

SymbolTable* scopeSymbolTable(SymbolTable *t);

SYMBOL* putSymbol(SymbolTable *t, char *name, void * value);

SYMBOL* getSymbol(SymbolTable *t, char *name);

/* This is used to find the SYMBOL and increment the counter,
 * to know how many static links should be travered to find the frame */
SYMBOL* getSymbolCount(SymbolTable *t, char *name, int* count);

void dumpSymbolTable(SymbolTable *t);


