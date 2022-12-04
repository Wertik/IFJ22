OBJS=main.o token.o scanner.o buffer.o symtable.o stack.o symbol.o parser.o instruction.o preparser.o
HEADER=scanner.h token.h buffer.h symtable.h symbol.h stack.h parser.h utils.h instruction.h preparser.h
OUT=main sym stack instr
CC=gcc
CFLAGS=--std=c99 -pedantic -Wall -Werror
LFLAGS=-lm
LEADER=xdobes22

.PHONY: all run clean pack main instr sym stack

all: main

main: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

sym: symtable_test.o symtable.o token.o instruction.o
	$(CC) -o $@ $^ $(LFLAGS)

stack: stack_test.o stack.o token.o symbol.o
	$(CC) -o $@ $^ $(LFLAGS)

instr: instruction_test.o instruction.o buffer.o
	$(CC) -o $@ $^ $(LFLAGS)

run: $(OUT)
	./$(OUT)

valgrind: $(OUT)
	valgrind --leak-check=full ./$(OUT)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $< -DVERBOSE

# Pack for submission
pack: all
	@rm $(LEADER).tgz -f
	tar -cvzf $(LEADER).tgz rozdeleni *.c Makefile *.h
	make clean

clean:
	@rm $(OBJS) -rf
	@rm stack_test.o -rf
	@rm symtable_test.o -rf
	@rm instruction_test.o -rf
	@rm $(OUT) -rf

