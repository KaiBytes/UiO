#clean:
	#rm live *.0

#live: live.c string.o
	#gcc $^ -o $0
	#This is for linking!

Cflags = -std=gnu11 -g -Wall -Wextra
Vflags = --track-origins=yes --leak-check=full --show-leak-kinds=all

all: oblig1 oblig1_tests

oblig1: oblig1.c
	gcc oppgave1.c -std=gnu11 -g -o oppgave1

oblig1_tests: oblig1_tests.c oppgave2.c
	gcc $(Cflags) oblig1_tests.c oppgave2.c -o oppgave2

check: oppgave2
	valgrind $(Vflags) ./oppgave2

clean:
	rm -f oppgave1 oppgave2