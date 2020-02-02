CC=gcc
GCC=gcc
CFLAGS=-std=c11 -g -static                      # Cコンパイラ引数
SRCS=$(filter-out yncc_test.c program.c, $(wildcard *.c))   # ソースリスト wildcardで拡張子がcのファイルのみを選択
OBJS=$(SRCS:.c=.o)				                # .cを.oに置換

make: $(OBJS)
	mv *.o ../obj

$(OBJS): parse.h

clean:
	rm -f *.o *~ tmp*

.PHONY: clean
