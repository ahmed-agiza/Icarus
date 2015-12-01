BUILD_DIR = ./build
SRC_DIR = ./src
INCLUDE_DIR = ./include
OBJ_DIR = ./obj
EX_LIB_DIR = /usr/lib
INCLUDE = -I$(INCLUDE_DIR)/
EX_LIB = -L$(EX_LIB_DIR)

SOBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/job.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/server.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/crypto.o\
 $(OBJ_DIR)/client_node.o\
 $(OBJ_DIR)/file.o\
 $(OBJ_DIR)/server_main.o

SDOBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/job.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/server.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/crypto.o\
 $(OBJ_DIR)/client_node.o\
 $(OBJ_DIR)/file.o\
 $(OBJ_DIR)/seeder.o\
 $(OBJ_DIR)/seeder_main.o

COBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/client.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/crypto.o\
 $(OBJ_DIR)/file.o\
 $(OBJ_DIR)/client_main.o

BINARIES = $(BUILD_DIR)/server $(BUILD_DIR)/client $(BUILD_DIR)/seeder
CC = g++
DEBUG = -g
LIBS = -pthread -lssl -lcrypto -ldl
CFLAGS = -Wall $(INCLUDE) $(EX_LIB) $(LIBS) -c $(DEBUG)
LFLAGS = -Wall $(INCLUDE) $(EX_LIB) $(LIBS) $(DEBUG)

all : clean server client seeder

server : $(SOBJS)
	$(CC) $(SOBJS) -o $(BUILD_DIR)/server $(LFLAGS)

client : $(COBJS)
	$(CC) $(COBJS) -o $(BUILD_DIR)/client $(LFLAGS)

seeder : $(SDOBJS)
	$(CC) $(SDOBJS) -o $(BUILD_DIR)/seeder $(LFLAGS)

$(OBJ_DIR)/file.o : $(OBJ_DIR)/udp_socket.o
	$(CC) -c $(SRC_DIR)/file.cpp -o $(OBJ_DIR)/file.o $(LFLAGS)

$(OBJ_DIR)/udp_socket.o : $(OBJ_DIR)/message.o $(OBJ_DIR)/thread.o
	$(CC) -c $(SRC_DIR)/udp_socket.cpp -o $(OBJ_DIR)/udp_socket.o $(LFLAGS)

$(OBJ_DIR)/job.o : $(OBJ_DIR)/thread.o
	$(CC) -c $(SRC_DIR)/job.cpp -o $(OBJ_DIR)/job.o $(LFLAGS)

$(OBJ_DIR)/thread.o : $(OBJ_DIR)/logger.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/thread.cpp -o $(OBJ_DIR)/thread.o $(LFLAGS)

$(OBJ_DIR)/thread_pool.o : $(OBJ_DIR)/thread.o $(OBJ_DIR)/logger.o
	$(CC) -c $(SRC_DIR)/thread_pool.cpp -o $(OBJ_DIR)/thread_pool.o $(LFLAGS)

$(OBJ_DIR)/message.o : $(OBJ_DIR)/logger.o $(OBJ_DIR)/crypto.o
	$(CC) -c $(SRC_DIR)/message.cpp -o $(OBJ_DIR)/logger.o $(LFLAGS)

$(OBJ_DIR)/client_node.o : $(OBJ_DIR)/logger.o
	$(CC) -c $(SRC_DIR)/client_node.cpp -o $(OBJ_DIR)/client_node.o $(LFLAGS)

$(OBJ_DIR)/logger.o : $(OBJ_DIR)/settings.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/logger.cpp -o $(OBJ_DIR)/message.o $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(LFLAGS)

clean:
	rm -f $(BINARIES) *.o $(BUILD_DIR)/* $(OBJ_DIR)/*.o $(BUILD_DIR)/core $(OBJ_DIR)/core
