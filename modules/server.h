#ifndef SERVER_H
#define SERVER_H

#include "server_socket.h"

class Server {
private:
  ServerSocket * _serverSocket;
  uint16_t _listenPort;
  const char *_getRawRequest();
  Message _getMessage();
  void _sendReply();
  ssize_t _acknowledge(const char *request);
  bool _acknowledgeAndWait(const char *request);
  //Message * getRequest();
  //Message * doOperation();
  //void sendReply (Message * _message);
  friend void *requestHandler(void *connection);



public:
  Server(uint16_t listenPort);
  void listen();

  void serveRequest(const char *request);
  ~Server();


  class ClientConnection {
    ServerSocket *_socket;
    sockaddr_in _clientAddr;
  public:
    ClientConnection(ServerSocket *handlerSocket):_socket(handlerSocket), _clientAddr(handlerSocket->getClientAddress()){}

    ServerSocket *getSocket() const {
      return _socket;
    }


    void setClientAddr (sockaddr_in clientAddr) {
      _clientAddr = clientAddr;
    }

    sockaddr_in getClienAddr() const {
      return _clientAddr;
    }

    ~ClientConnection(){
      if(_socket){
        delete _socket;
      }
    }
  };
};
#endif // SERVER_H
