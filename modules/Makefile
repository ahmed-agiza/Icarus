SOBJS = server_main.o udp_socket.o server_socket.o message.o server.o
COBJS = client_main.o udp_socket.o client_socket.o message.o client.o
BINARIES = server client
CC = g++
DEBUG = -g
CFLAGS = -Wall -pthread -c $(DEBUG)
LFLAGS = -Wall -pthread $(DEBUG)

all : server client

server : $(SOBJS)
	$(CC) $(LFLAGS) $(SOBJS) -o server

client : $(COBJS)
	$(CC) $(LFLAGS) $(COBJS) -o client

server_socket.o : udp_socket.o
	$(CC) $(LFLAGS) -c server_socket.cpp

client_socket.o : udp_socket.o
	$(CC) $(LFLAGS) -c client_socket.cpp

udp_socket.o : message.o
	$(CC) $(LFLAGS) -c udp_socket.cpp

clean:
	rm -f $(BINARIES) *.o
