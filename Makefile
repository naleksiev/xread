.build/test.o: test/test.c src/xread.h src/xread.c
	mkdir -p .build
	gcc -Os test/test.c src/xread.c -o .build/test.o

all: .build/test.o

clean:
	rm -Rf .build

run: .build/test.o
	@if .build/test.o ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;

