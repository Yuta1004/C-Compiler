GCC=gcc
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

yncc: $(OBJS)
	$(GCC) -o yncc $(OBJS) $(LDFLAGS)

$(OBJS): yncc.h

test: yncc
	./test.sh

clean:
	rm -f yncc *.o *~ tmp*

.PHONY: test clean
