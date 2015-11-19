#ifndef CLIENT_H
#define CLIENT_H

#include "udp_socket.h"

#define MAX_RETRY 3
#define CLIENT_REPLY_TO 3

class Client {
private:
  UDPSocket * _clientSocket;
  uint16_t _port;
  char _hostname[128];
  Message _getReply();
  Message _getReplyTimeout(time_t seconds = 0, suseconds_t mseconds = 0);
  void _establishConnection(); //connect to server
  ssize_t _sendMessage(Message message);

public:
  Client(const char * hostname, uint16_t port);
  int start(); //
  //Message * execute(Message * _message);
  ~Client();
};
#endif // CLIENT_H
