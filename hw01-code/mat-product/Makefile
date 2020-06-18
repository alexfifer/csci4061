# Makefile for mat-product
CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

all : op_main

op_main.o : op_main.c mat.h
	$(CC) -c op_main.c

outer_product.o : outer_product.c mat.h
	$(CC) -c $<


free_matrix.o : free_matrix.c mat.h
	$(CC) -c $<

test : op_main
	@chmod u+x testy
	./testy test_outer_product.org $(testnum)


op_main : op_main.o outer_product.o free_matrix.o
	$(CC) -o op_main free_matrix.o outer_product.o op_main.o
	@echo op_main is ready

clean:
	rm -f op_main test_collatz *.o

