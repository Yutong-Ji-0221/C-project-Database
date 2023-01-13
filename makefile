CC=gcc
CFLAGS= -g -fPIC -pedantic -pthread -Wall -std=gnu99

.PHONY: all clean
.DEFAULT_GOAL := all
all: host client

host: host.o auxiliary.o database.o lock.o
		$(CC) $(CFLAGS) $^ -o $@

client: client.o auxiliary.o
		$(CC) $(CFLAGS) $^ -o $@

host.o: host.c host.h auxiliary.h database.h strcuts.h lock.h
auxiliary.o: auxiliary.c auxiliary.h
database.o: database.c database.h strcuts.h auxiliary.h
lock.o: lock.c lock.h
client.o: client.c client.h auxiliary.h

clean:
	rm host client *.o