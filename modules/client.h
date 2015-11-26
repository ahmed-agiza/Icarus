#ifndef CLIENT_H
#define CLIENT_H

#include "client_socket.h"

#define MAX_RETRY 3

class Client {
private:
  ClientSocket * _clientSocket;
  uint16_t _port;
  char _hostname[128];
  const char * _getRawReply();
  const char * _getRawReplyTimeout(time_t seconds = 0, suseconds_t micro = 0);
  Message _getReply();
  void _establishConnection(); //connect to server
  ssize_t _sendRawMessage(char * m);
  ssize_t _sendMessage(Message message);

public:
  Client(char * hostname, uint16_t port);
  int start(); //


  //Message * execute(Message * _message);
  ~Client();
};
#endif // CLIENT_H
