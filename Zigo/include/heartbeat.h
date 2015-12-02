#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "thread.h"
#include "udp_socket.h"
#include "file.h"
#include "crypto.h"
#include "seeder_node.h"

enum ConnectionState {
  Connected = 0,
  Disconnected = 1
};

enum Operation {
  Pinging = 0,
  Querying = 1
};

enum State {
  Steady = 0,
  Fetching = 1,
  Ready = 2
};

class HeartBeat : public Thread {
  UDPSocket * _clientSocket;
  uint16_t _port;
  char _hostname[128];
  Message _getReply();
  Message _getReplyTimeout(time_t seconds = 0, suseconds_t mseconds = 0);
  void _establishConnection(); //connect to server
  ssize_t _sendMessage(Message message);
  ConnectionState _state;
  char _publicRSA[2048];
  char _privateRSA[2048];
  char _id[128];
  char _results[MAX_READ_SIZE];
  Operation _currentOperation;
  State _resultState;
  pthread_mutex_t _operationLock;
  pthread_mutex_t _fetchingCvLock;
  pthread_cond_t _fetchingCv;
public:
  HeartBeat(const char *hostname, uint16_t port);
  void run();
  bool reset();
  void stop();

  void queryOnline();
  void fetchResults(char *buf);


  //Message * execute(Message * _message);
  ~HeartBeat();
};

#endif //HEARTBEAT_H
