SOBJS = server_main.o thread.o thread_pool.o job.o udp_socket.o message.o server.o logger.o
COBJS = client_main.o thread.o thread_pool.o udp_socket.o message.o client.o logger.o
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

job.o : thread.o
	$(CC) $(LFLAGS) -c job.cpp

thread.o : logger.o
	$(CC) $(LFLAGS) -c thread.cpp

thread_pool.o : thread.o logger.o
	$(CC) $(LFLAGS) -c thread_pool.cpp

message.o : logger.o
	$(CC) $(LFLAGS) -c message.cpp

logger.o : settings.o
	$(CC) $(LFLAGS) -c logger.cpp

clean:
	rm -f $(BINARIES) *.o
