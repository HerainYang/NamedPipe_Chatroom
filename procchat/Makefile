CC=gcc
CFLAGS=-Wall -Werror -Wvla -std=c11 -fsanitize=address
PFLAGS=-fprofile-arcs -ftest-coverage
DFLAGS=-g
HEADERS=server.h serverOperation.h
SRC=server.c serverOperation.c

procchat: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -std=gnu99 -o $@

test: procchat
	./test.sh

clean:
	rm -f procchat clientHandler

