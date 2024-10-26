CFLAGS = -g -ansi -Wall -pedantic
utility_headers = enums.h structs.h ext_vars.h utils.h

assembler: utils.o first_pass.o second_pass.o main.o
	gcc $(CFLAGS) utils.o first_pass.o main.o -lm -o assembler

utils.o: utils.c $(utility_headers) utils.h
	gcc $(CFLAGS) utils.c -o utils.o

preassembler.o: preassembler.c $(utility_headers) preassembler.h
	gcc $(CFLAGS) preassembler.c -o preassembler.o

first_pass.o: first_pass.c $(utility_headers) first_pass.h
	gcc $(CFLAGS) first_pass.c -o first_pass.o

second_pass.o: second_pass.c $(utility_headers) second_pass.h
	gcc $(CFLAGS) second_pass.c -o second_pass.o

main.o: main.c $(utility_headers) first_pass.h second_pass.h
	gcc $(CFLAGS) main.c -o main.o