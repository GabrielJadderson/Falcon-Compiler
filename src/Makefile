
# the project/executeable name
TARGET=compiler

# the compiler
CC=gcc

# compile flags
CFLAGS = -std=c11 -Wall -Wextra -pedantic


DEP=tree.h tree.o pretty.h pretty.o memory.h memory.o falcon_arguments.h falcon_arguments.o file_io.h file_io.o falcon_string.h falcon_string.o logger.h logger.o linked_list.h linked_list.o array.h array.o stack.h stack.o symbol.h symbol.o code_gen.h code_gen.o code_gen_util.h code_gen_util.o weeder.h weeder.o type_checker.h type_checker.o
OBJECTS=tree.o pretty.o memory.o file_io.o symbol.o weeder.o type_checker.o code_gen_util.o code_gen.o falcon_arguments.o logger.o falcon_string.o array.o linked_list.o stack.o


main: y.tab.o lex.yy.o main.o $(DEP)
	$(CC) $(CFLAGS) lex.yy.o y.tab.o $(OBJECTS) main.o -o $(TARGET) -lfl

y.tab.c y.tab.h: exp.y
	bison -y -d exp.y

lex.yy.c: exp.l y.tab.h
	flex exp.l

.PHONY: compileASM
compileASM:
	gcc -g -no-pie -m64 output.s

.PHONY: clean
clean:
	rm -f *.o y.tab.c lex.yy.c y.tab.h compiler
	@echo "Cleanup complete!"

