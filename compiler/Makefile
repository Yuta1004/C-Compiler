GCC=gcc
CFLAGS=-std=c11 -g -static      # Cコンパイラ引数
SRCS=$(wildcard *.c)			# ソースリスト wildcardで拡張子がcのファイルのみを選択
OBJS=$(SRCS:.c=.o)				# .cを.oに置換

yncc: $(OBJS)
	$(GCC) -o yncc $(OBJS) $(LDFLAGS)

$(OBJS): yncc.h

test: yncc
	./test.sh

clean:
	rm -f yncc *.o *~ tmp*

.PHONY: test clean
