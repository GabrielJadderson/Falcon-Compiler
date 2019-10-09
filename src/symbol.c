#include "symbol.h"
#include "memory.h"
#include <string.h>
#include <stdio.h>
#include "type_checker.h"
#include "logger.h"


unsigned int Hash(char* str)
{
	unsigned int length = strlen(str);
	if (!length) return 0;
	if (length > 31) length = 31;
	unsigned int value = 0;
	for (size_t i = 0; i < length - 1; i++)
	{
		value += str[i];
		value = value << 1;
	}
	value += str[length - 1];
	return value;
}

SymbolTable* initSymbolTable()
{
	SymbolTable* st = Calloc(1, sizeof(struct SymbolTable));
	return st;
}

SymbolTable* scopeSymbolTable(SymbolTable* t)
{
	SymbolTable* st = initSymbolTable();
	st->next = t;
	return st;
}

SYMBOL* putSymbol(SymbolTable* t, char* name, void *value)
{
	if (t)
	{
		int index = Hash(name) % HashSize;
		SYMBOL* s = Calloc(1, sizeof(struct SYMBOL));
		s->name = name;
		s->value = value;

		SYMBOL* temp = t->table[index];

		if (!temp)
		{
			t->table[index] = s;
			return s;
		}

		while (temp)
		{
			if (strcmp(temp->name, name) == 0)
			{
				//printf("================ [SYMBOL_TABLE] OVERWRITTEN the value of %s |from=%d to=%d.\n", temp->name, ((type_info*)temp->value)->kind, ((type_info*)value)->kind);
				//Name duplication, overwrites existing value with new value
				temp->value = value;
				return temp;
			}
			if (!temp->next)
			{
				temp->next = s;
				return s;
			}
			temp = temp->next;
		}
	}
	return 0;
}

SYMBOL* getSymbol(SymbolTable* t, char* name)
{
	if (t)
	{
		int index = Hash(name) % HashSize;
		SYMBOL* s = t->table[index];
		SymbolTable* hashtable = t;
		while (hashtable != NULL)
		{
			s = hashtable->table[index];
			while (s != NULL)
			{
				if (strcmp(s->name, name) == 0)
				{
					return s;
				}
				s = s->next;
			}
			hashtable = hashtable->next;
		}
	}



	return NULL;
}

SYMBOL* getSymbolCount(SymbolTable* t, char* name, int* count)
{
	if (t)
	{
		int index = Hash(name) % HashSize;
		SYMBOL* s = t->table[index];
		SymbolTable* hashtable = t;
		while (hashtable != NULL)
		{
			s = hashtable->table[index];
			while (s != NULL)
			{
				if (strcmp(s->name, name) == 0)
				{
					return s;
				}
				s = s->next;
			}
			hashtable = hashtable->next;

			//increment foreach time new SymbolTable is looked at
			*count += 1;
		}
	}

	return NULL;
}

void recursively_print_kind(type_info* type)
{
	if (type)
	{
		printf(" -> %s:%d", type->type_id, type->kind);
		if (type->type_info_child)
		{
			recursively_print_kind(type->type_info_child);
		}
	}
}


void dumpSymbolTable(SymbolTable* t)
{

	SYMBOL* s = NULL;
	SymbolTable* hashtable = t;

	while (hashtable != NULL)
	{
		for (int index = 0; index < HashSize; index++)
		{
			s = hashtable->table[index];
			if (s)
			{
				printf(" |%s", s->name);
				printf(":%d", ((type_info*)s->value)->kind);
				if (((type_info*)s->value)->type_info_child)
				{
					recursively_print_kind(((type_info*)s->value)->type_info_child);
				}
				printf("| ");
				//printf(" | %s : %d | ", s->name, ((type_info*)s->value)->kind);
				while (s->next)
				{
					//printf(" | %s : %d | ", s->name, ((type_info*)s->value)->kind);
					printf(" |%s", s->name);
					printf(":%d", ((type_info*)s->value)->kind);
					if (((type_info*)s->value)->type_info_child)
					{
						recursively_print_kind(((type_info*)s->value)->type_info_child);
					}
					printf("| ");
					s = s->next;
				}

			}
		}
		printf("\n");
		hashtable = hashtable->next;
	}
}