CC=gcc
GCC=gcc
CFLAGS=-std=c11 -g -static                      # Cコンパイラ引数
SRCS=$(filter-out yncc_test.c program.c, $(wildcard *.c))   # ソースリスト wildcardで拡張子がcのファイルのみを選択
OBJS=$(SRCS:.c=.o)				                # .cを.oに置換
TARGET=yncc_test.c

yncc: $(OBJS)
	$(GCC) -o yncc $(OBJS) $(LDFLAGS)

$(OBJS): yncc.h

test:
	make test-c

test-sh: yncc
	./test.sh

test-c: yncc $(TARGET)
	./yncc $(TARGET) > tmp.s
	$(GCC) -g3 -no-pie -o tmp tmp.s
	./tmp

lines:
	@ wc -l $(SRCS) | head -n -1 | sort -k 1 -r -n
	@ wc -l $(SRCS) | tail -n 1

clean:
	rm -f *.o *~ tmp*

.PHONY: test clean
