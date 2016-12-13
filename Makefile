test.o: test/test.c src/xread.h src/xread.c
	gcc test/test.c src/xread.c -o test.o

all: test.o

clean:
	rm -f test.o

test: test.o
	@if ./test.o ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;

