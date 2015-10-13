#ifndef SERVER_H
#define SERVER_H

#include "server_socket.h"

class Server {
private:
  ServerSocket * _serverSocket;
  uint16_t _listenPort;
  char *_getRequest();
  void _sendReply();
  //Message * getRequest();
  //Message * doOperation();
  //void sendReply (Message * _message);
public:
  Server(uint16_t listenPort);
  void listen();

  //void serveRequest();
  ~Server();
};
#endif // SERVER_H
