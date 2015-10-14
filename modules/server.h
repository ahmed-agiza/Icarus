#ifndef SERVER_H
#define SERVER_H

#include "server_socket.h"

class Server {
private:
  ServerSocket * _serverSocket;
  uint16_t _listenPort;
  const char *_getRequest();
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
    Server *_parent;
    const char* _request;
    sockaddr_in _clientAddr;
  public:
    ClientConnection(Server *parent, const char *request, sockaddr_in clientAddr):_parent(parent), _request(request), _clientAddr(clientAddr){}

    Server *getServer() const {
      return _parent;
    }

    void setRequest(char *request){
      _request = request;
    }
    const char *getRequest() const {
      return _request;
    }

    void setClientAddr (sockaddr_in clientAddr) {
      _clientAddr = clientAddr;
    }

    sockaddr_in getClienAddr() const {
      return _clientAddr;
    }

    ~ClientConnection(){
      if (_request)
        delete _request;
    }
  };
};
#endif // SERVER_H
