CXX=gcc
CFLAGS=-Wall -pedantic -O3
LFLAGS=-lm

all: test

doc:
	doxygen

%.o: %.c
	$(CXX) -c $(CFLAGS) $^ -o $@

test: test.o parse_json.o lex_json.o
	$(CXX) $^ -o $@ $(LFLAGS)

install: parse_json.o lex_json.o
	ar r libparse_json.a $^
	mv libparse_json.a /usr/local/lib/
	cp lex_json.h parse_json.h /usr/local/include

clean:
	rm *.a *.o test
