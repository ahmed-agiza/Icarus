#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "thread.h"
#include "udp_socket.h"
#include "client_node.h"

enum State {
  Connected,
  Disconnected
};

class HeartBeat : public Thread {
  UDPSocket * _clientSocket;
  uint16_t _port;
  char _hostname[128];
  Message _getReply();
  Message _getReplyTimeout(time_t seconds = 0, suseconds_t mseconds = 0);
  void _establishConnection(); //connect to server
  ssize_t _sendMessage(Message message);
  State _state;
public:
  HeartBeat(const char * hostname, uint16_t port);
  void run(); //
  bool reset();
  void stop();


  //Message * execute(Message * _message);
  ~HeartBeat();
};

#endif //HEARTBEAT_H
