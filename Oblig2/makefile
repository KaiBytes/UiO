Cflags = -std=gnu11 -g -Wall -Wextra
Vflags = --track-origins=yes --leak-check=full --show-leak-kinds=all

all: ruterdrift

ruterdrift: ruterdrift.c 
	gcc $(Cflags) ruterdrift.c -o ruterdrift

checkFive: ruterdrift
	valgrind $(Vflags) ./ruterdrift testing_data/5_routers_fully_connected/topology.dat

checkTen: ruterdrift
	valgrind $(Vflags) ./ruterdrift testing_data/10_routers_15_edges/topology.dat testing_data/10_routers_15_edges/commands.txt

checkFifteen: ruterdrift
	valgrind $(Vflags) ./ruterdrift testing_data/50_routers_150_edges/topology.dat testing_data/50_routers_150_edges/commands.txt

clean:
	rm -f ruterdrift new-topology.dat