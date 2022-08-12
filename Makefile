CC=gcc
CFLAGS=-Wall -pedantic -O3
LDLIBS=-lm

all: test

doc:
	doxygen

#%.o: %.c
#	$(CC) -c $(CFLAGS) $^ -o $@

test: test.o parse_json.o lex_json.o
	$(CC) $^ -o $@ $(LDLIBS)

install: parse_json.o lex_json.o
	ar r libparse_json.a $^
	mv libparse_json.a /usr/local/lib/
	cp lex_json.h parse_json.h /usr/local/include

clean:
	rm *.o test
