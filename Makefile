all: client server
	
client: client.o
	cc -g -o client client.o

server: server.o
	cc -g -o server server.o

client.o: client.c
	cc -c -Wall -g client.c

server.o: server.c
	cc -c -Wall -g server.c

clean:
	rm client server client.o server.o
