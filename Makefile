CC=cc
CFLAGS=-std=c23 -g -Wall -Wextra -Wpedantic -fsanitize=address 

.PHONY: all
all: procress.so procress.a test

clean:
	rm -f *.o *.so *.a test

procress.o: procress.c procress.h
	$(CC) $(CFLAGS) -c -o $@ $<

procress.so: procress.o
	$(CC) -fpic -shared -o $@ $< 

procress.a: procress.o
	ar r procress.a $^

test: test.c procress.a
	$(CC) $(CFLAGS) -g -o $@ $^
