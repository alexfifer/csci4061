# See Makefile-commented for explanation
CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

all : collatz_main


collatz_main.o : collatz_main.c collatz.h
	$(CC) -c collatz_main.c

collatz_funcs.o : collatz_funcs.c collatz.h
	$(CC) -c $<

test_collatz : test_collatz.c collatz_funcs.o collatz.h
	$(CC) -o $@ $^

test : test_collatz collatz_main
	@chmod u+x testy
	./testy test_collatz.org $(testnum)

collatz_main : collatz_funcs.o collatz_main.o
	$(CC) -o collatz_main collatz_funcs.o collatz_main.o
	@echo collatz_main is ready

clean:
	rm -f collatz_main test_collatz *.o

clean-tests:
	@echo Removing temporary test files
	rm -f *.o test_collatz test-results

