CFLAGS = -g -Wall -Wextra -pedantic -Wno-unused-parameter
CC = gcc

PROGRAMS = testMain myGit

all : $(PROGRAMS)

testMain : main.o hash.o list.o work.o
	$(CC)  -o $@ $(CFLAGS) $^

myGit : myGit.o commit.o list.o hash.o work.o
	$(CC)  -o $@ $(CFLAGS) $^

work.o : work.c
	$(CC) -c work.c

hash.o : hash.c
	$(CC) -c hash.c

list.o : list.c
	$(CC) -c list.c

commit.o : commit.c
	$(CC) -c commit.c

main.o : main.c
	$(CC) -c main.c

clean : 
	rm -rf *.o *~ $(PROGRAMS) */* */ .refs/ .current_branch .add testMain.txt testMainCopy.txt