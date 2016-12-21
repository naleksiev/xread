.build/test.o: test/test.c src/xread.h src/xread.c
	mkdir -p .build
	$(CC) -Os test/test.c src/xread.c -o .build/test.o

all: .build/test.o

clean:
	rm -Rf .build

run: .build/test.o
	@if .build/test.o test/test.xml ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;

