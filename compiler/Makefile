GCC=gcc
CFLAGS=-std=c11 -g -static                      # Cコンパイラ引数
SRCS=$(filter-out yncc_test.c program.c, $(wildcard *.c))   # ソースリスト wildcardで拡張子がcのファイルのみを選択
OBJS=$(SRCS:.c=.o)				                # .cを.oに置換

yncc: $(OBJS)
	$(GCC) -o yncc $(OBJS) $(LDFLAGS)

$(OBJS): yncc.h

test:
	make test-c

test-sh: yncc
	./test.sh

test-c: yncc yncc_test.c
	./yncc yncc_test.c > tmp.s
	$(GCC) -no-pie -o tmp tmp.s
	./tmp

run-program: yncc program.c
	./yncc program.c > tmp.s
	$(GCC) -no-pie -o tmp tmp.s
	./tmp

lines:
	wc -l $(SRCS)

clean:
	rm -f *.o *~ tmp*

.PHONY: test clean
