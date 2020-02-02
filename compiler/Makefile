CC=gcc
GCC=gcc
CFLAGS=-std=c11 -g -static                      # Cコンパイラ引数
SRCS=$(filter-out $(wildcard test/*.c) program.c, $(wildcard *.c **/*.c))   # ソースリスト wildcardで拡張子がcのファイルのみを選択
OBJS=$(SRCS:.c=.o)
TARGET=test/yncc_test.c

yncc: $(OBJS)
	$(GCC) -o yncc $(OBJS) $(LDFLAGS)

test:
	make test-c

test-sh: yncc
	./test/test.sh

test-c: yncc $(TARGET)
	./yncc $(TARGET) > tmp.s
	$(GCC) -g3 -no-pie -o tmp tmp.s
	./tmp

lines:
	@ wc -l $(SRCS) | head -n -1 | sort -k 1 -r -n
	@ wc -l $(SRCS) | tail -n 1

clean:
	rm -f *~ tmp* *.o **/*.o

.PHONY: test clean
