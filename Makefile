OBJS=main.o token.o scanner.o string.o array.o symtable.o stack.o grammar.o parser.o
HEADER=scanner.h token.h string.h array.h symtable.h grammar.h stack.h parser.h
OUT=main
CC=gcc
CFLAGS=--std=c99 -pedantic -Wall -Werror
LFLAGS=-lm
LEADER=xdobes02

.PHONY: all run clean pack main

all: main

main: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

btree: btree_test.o symtable.o
	$(CC) -o $@ $^ $(LFLAGS)

stack: stack_test.o stack.o token.o grammar.o
	$(CC) -o $@ $^ $(LFLAGS)

run: $(OUT)
	./$(OUT)

valgrind: $(OUT)
	valgrind --leak-check=full ./$(OUT)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

# Pack for submission
pack: all
	@rm $(LEADER).tgz -f
	tar -cvzf $(LEADER).tgz rozdeleni *.c
	make clean

clean:
	@rm $(OBJS) -rf
	@rm $(OUT) -rf

