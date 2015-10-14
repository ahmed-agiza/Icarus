#ifndef CLIENT_H
#define CLIENT_H

#include "client_socket.h"

class Client {
private:
  ClientSocket * _clientSocket;
  uint16_t _port;
  char _hostname[128];
  const char * _getReply();
  void _establishConnection();
public:
  Client(char * hostname, uint16_t port);
  int start();
  ssize_t _sendMessage(char * m);

  //Message * execute(Message * _message);
  ~Client();
};
#endif // CLIENT_H
