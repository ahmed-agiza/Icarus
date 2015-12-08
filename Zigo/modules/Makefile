.PHONY: all server client seeder app pre

BUILD_DIR = ./build
SRC_DIR = ./src
INCLUDE_DIR = ./include
OBJ_DIR = ./obj
EX_LIB_DIR = /usr/lib
INCLUDE = -I$(INCLUDE_DIR)/
EX_LIB = -L$(EX_LIB_DIR)
STEGHIDE_EXISTS = $(shell which steghide)

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
 $(OBJ_DIR)/peer.o\
 $(OBJ_DIR)/steganography.o\
 $(OBJ_DIR)/server_main.o

SDOBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/seeder_job.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/crypto.o\
 $(OBJ_DIR)/seeder_node.o\
 $(OBJ_DIR)/file.o\
 $(OBJ_DIR)/seeder.o\
 $(OBJ_DIR)/peer.o\
 $(OBJ_DIR)/seeder_main.o

COBJS = \
 $(OBJ_DIR)/thread.o\
 $(OBJ_DIR)/thread_pool.o\
 $(OBJ_DIR)/udp_socket.o\
 $(OBJ_DIR)/heartbeat.o\
 $(OBJ_DIR)/message.o\
 $(OBJ_DIR)/client.o\
 $(OBJ_DIR)/logger.o\
 $(OBJ_DIR)/settings.o\
 $(OBJ_DIR)/crypto.o\
 $(OBJ_DIR)/file.o\
 $(OBJ_DIR)/peer.o\
 $(OBJ_DIR)/steganography.o\
 $(OBJ_DIR)/client_manager.o\
 $(OBJ_DIR)/client_main.o

APPOBJS = \
	$(OBJ_DIR)/thread.o\
  $(OBJ_DIR)/thread_pool.o\
  $(OBJ_DIR)/udp_socket.o\
  $(OBJ_DIR)/heartbeat.o\
  $(OBJ_DIR)/message.o\
  $(OBJ_DIR)/client.o\
  $(OBJ_DIR)/logger.o\
  $(OBJ_DIR)/settings.o\
  $(OBJ_DIR)/crypto.o\
  $(OBJ_DIR)/file.o\
  $(OBJ_DIR)/peer.o\
  $(OBJ_DIR)/job.o\
  $(OBJ_DIR)/server.o\
  $(OBJ_DIR)/client_node.o\
  $(OBJ_DIR)/steganography.o\
	$(OBJ_DIR)/client_manager.o\
	$(OBJ_DIR)/app.o


BINARIES = $(BUILD_DIR)/server $(BUILD_DIR)/client $(BUILD_DIR)/seeder $(BUILD_DIR)/app
CC = g++
DEBUG = -g
LIBS = -pthread -lssl -lcrypto -ldl
CFLAGS = -Wall -Wno-write-strings -Wno-reorder $(INCLUDE) $(EX_LIB) $(LIBS) -c $(DEBUG)
LFLAGS = -Wall -Wno-write-strings -Wno-reorder $(INCLUDE) $(EX_LIB) $(LIBS) $(DEBUG)

pre :
ifeq ($(STEGHIDE_EXISTS),)
	$(error cannot locate steghide)
	exit 1;
endif
	$(ulimit -c unlimited)

all : pre clean server client seeder app

app: pre $(APPOBJS)
	$(CC) $(APPOBJS) -o $(BUILD_DIR)/app $(LFLAGS)

server : pre $(SOBJS)
	$(CC) $(SOBJS) -o $(BUILD_DIR)/server $(LFLAGS)

client : pre $(COBJS)
	$(CC) $(COBJS) -o $(BUILD_DIR)/client $(LFLAGS)

seeder : pre $(SDOBJS)
	$(CC) $(SDOBJS) -o $(BUILD_DIR)/seeder $(LFLAGS)

$(OBJ_DIR)/steganography.o : $(OBJ_DIR)/file.o
	$(CC) -c $(SRC_DIR)/steganography.cpp -o $(OBJ_DIR)/steganography.o $(LFLAGS)

$(OBJ_DIR)/file.o : $(OBJ_DIR)/udp_socket.o
	$(CC) -c $(SRC_DIR)/file.cpp -o $(OBJ_DIR)/file.o $(LFLAGS)

$(OBJ_DIR)/udp_socket.o : $(OBJ_DIR)/message.o $(OBJ_DIR)/thread.o
	$(CC) -c $(SRC_DIR)/udp_socket.cpp -o $(OBJ_DIR)/udp_socket.o $(LFLAGS)

$(OBJ_DIR)/seeder_job.o : $(OBJ_DIR)/thread.o
	$(CC) -c $(SRC_DIR)/seeder_job.cpp -o $(OBJ_DIR)/seeder_job.o $(LFLAGS)

$(OBJ_DIR)/heartbeat.o : $(OBJ_DIR)/thread.o
	$(CC) -c $(SRC_DIR)/heartbeat.cpp -o $(OBJ_DIR)/heartbeat.o $(LFLAGS)

$(OBJ_DIR)/job.o : $(OBJ_DIR)/thread.o
	$(CC) -c $(SRC_DIR)/job.cpp -o $(OBJ_DIR)/job.o $(LFLAGS)

$(OBJ_DIR)/thread.o : $(OBJ_DIR)/logger.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/thread.cpp -o $(OBJ_DIR)/thread.o $(LFLAGS)

$(OBJ_DIR)/thread_pool.o : $(OBJ_DIR)/thread.o $(OBJ_DIR)/logger.o
	$(CC) -c $(SRC_DIR)/thread_pool.cpp -o $(OBJ_DIR)/thread_pool.o $(LFLAGS)

$(OBJ_DIR)/message.o : $(OBJ_DIR)/logger.o $(OBJ_DIR)/crypto.o
	$(CC) -c $(SRC_DIR)/message.cpp -o $(OBJ_DIR)/logger.o $(LFLAGS)

$(OBJ_DIR)/seeder_node.o : $(OBJ_DIR)/logger.o
	$(CC) -c $(SRC_DIR)/seeder_node.cpp -o $(OBJ_DIR)/seeder_node.o $(LFLAGS)

$(OBJ_DIR)/client_node.o : $(OBJ_DIR)/logger.o
	$(CC) -c $(SRC_DIR)/client_node.cpp -o $(OBJ_DIR)/client_node.o $(LFLAGS)

$(OBJ_DIR)/peer.o : $(OBJ_DIR)/logger.o
	$(CC) -c $(SRC_DIR)/peer.cpp -o $(OBJ_DIR)/peer.o $(LFLAGS)

$(OBJ_DIR)/logger.o : $(OBJ_DIR)/settings.o
	$(CC) $(LFLAGS) -c $(SRC_DIR)/logger.cpp -o $(OBJ_DIR)/message.o $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ $(LFLAGS)

clean:
	rm -f $(BINARIES) *.o $(OBJ_DIR)/*.o $(BUILD_DIR)/core $(OBJ_DIR)/core
