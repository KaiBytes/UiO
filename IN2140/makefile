Cflags = -std=gnu11 -g -Wall -Wextra
Vflags = --track-origins=yes --leak-check=full --show-leak-kinds=all
BIN = client server cl
ARGCLIENT = 127.0.0.1 2022 0.1
ARGSERVER = 2022 H1-Multiplexing-and-Loss-Recovery.pdf 2 0.1

all: $(BIN)


client: client.o rdp.o send_packet.o
	gcc $(Cflags) client.o rdp.o send_packet.o -o client

client.o: client.c
	gcc $(Cflags) client.c -c

server: server.o rdp.o send_packet.o
	gcc $(Cflags) server.c rdp.o send_packet.o -o server

server.o: server.c
	gcc $(Cflags) server.c -c

send_packet.o: send_packet.c send_packet.h
	gcc $(Cflags) send_packet.c -c

rdp: rdp.o send_packet.o
	gcc $(Cflags) rdp.o send_packet.o -o rdp

rdp.o: rdp.c rdp.h
	gcc $(Cflags) rdp.c -c

checkC: client
	valgrind $(Vflags) ./client ${ARGCLIENT}

checkS: server
	valgrind $(Vflags) ./server ${ARGSERVER}

clean:
	rm -f *.pdf *o $(BIN) 

