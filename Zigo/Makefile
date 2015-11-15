SOBJS = server_main.o thread.o job.o udp_socket.o message.o server.o
COBJS = client_main.o thread.o udp_socket.o message.o client.o
BINARIES = server client
CC = g++
DEBUG = -g
CFLAGS = -Wall -pthread -c $(DEBUG)
LFLAGS = -Wall -pthread $(DEBUG)

all : clean server client

server : $(SOBJS)
	$(CC) $(LFLAGS) $(SOBJS) -o server

client : $(COBJS)
	$(CC) $(LFLAGS) $(COBJS) -o client

udp_socket.o : message.o thread.o
	$(CC) $(LFLAGS) -c udp_socket.cpp

clean:
	rm -f $(BINARIES) *.o
