BUILD_DIR = ./build
SRC_DIR = ./src
INCLUDE_DIR = ./include
OBJ_DIR = ./obj
INCLUDE = -I$(INCLUDE_DIR)/

SOBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/job.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/server.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/client_node.o\
 $(OBJ_DIR)/server_main.o

COBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/client.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/client_main.o

BINARIES = $(BUILD_DIR)/server $(BUILD_DIR)/client
CC = g++
DEBUG = -g
LIBS = -pthread
CFLAGS = -Wall $(INCLUDE) $(LIBS) -c $(DEBUG)
LFLAGS = -Wall $(INCLUDE) $(LIBS) $(DEBUG)

all : clean server client

server : $(SOBJS)
	$(CC) $(LFLAGS) $(SOBJS) -o $(BUILD_DIR)/server

client : $(COBJS)
	$(CC) $(LFLAGS) $(COBJS) -o $(BUILD_DIR)/client

$(OBJ_DIR)/udp_socket.o : $(OBJ_DIR)/message.o $(OBJ_DIR)/thread.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/udp_socket.cpp -o $(OBJ_DIR)/udp_socket.o

$(OBJ_DIR)/job.o : $(OBJ_DIR)/thread.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/job.cpp -o $(OBJ_DIR)/job.o

$(OBJ_DIR)/thread.o : $(OBJ_DIR)/logger.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/thread.cpp -o $(OBJ_DIR)/thread.o

$(OBJ_DIR)/thread_pool.o : $(OBJ_DIR)/thread.o $(OBJ_DIR)/logger.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/thread_pool.cpp -o $(OBJ_DIR)/thread_pool.o

$(OBJ_DIR)/message.o : $(OBJ_DIR)/logger.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/message.cpp -o $(OBJ_DIR)/logger.o

$(OBJ_DIR)/client_node.o : $(OBJ_DIR)/logger.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/client_node.cpp -o $(OBJ_DIR)/client_node.o

$(OBJ_DIR)/logger.o : $(OBJ_DIR)/settings.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/logger.cpp -o $(OBJ_DIR)/message.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(LFLAGS)

clean:
	rm -f $(BINARIES) *.o $(BUILD_DIR)/* $(OBJ_DIR)/*.o $(BUILD_DIR)/core $(OBJ_DIR)/core
