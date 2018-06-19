ifndef CC
	CC=gcc
endif
CFLAGS=-std=c99 -Wall -Wpedantic -Wextra
SRCS=lookup.c
OBJS=$(subst .c,.o,$(SRCS))
RM=rm -f

all: lookup

foo: lookup.o
	$(CC) $(CFLAGS) -o lookup lookup.c

clean:
	$(RM) $(OBJS) lookup

