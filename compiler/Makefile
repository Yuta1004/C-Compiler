GCC = gcc
GCC_OPT = gcc -g -static

build:
	$(GCC_OPT) -o cc.o cc.c

build-tmp:
	$(GCC) -o tmp.o tmp.s

test:
	make build
	./test.sh

clean:
	rm -f *.o
