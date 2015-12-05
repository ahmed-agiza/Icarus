#include "server_socket.h"

ServerSocket::ServerSocket ():UDPSocket(){ //set socket options
  int socketOp = 1;
  setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, (void *)&socketOp, sizeof(socketOp));
}

bool ServerSocket::initializeServer (uint16_t hostPort){
  _hostAddr.sin_family = AF_INET;
  _hostAddr.sin_port = htons(hostPort);
  _hostAddr.sin_addr.s_addr = INADDR_ANY;
  bzero(&_hostAddr.sin_zero, 8);
  if(bind(_socketFd, (sockaddr *) &_hostAddr, sizeof(_hostAddr)) == -1)
    throw "Error binding server socket.\n";
  return 1;
}

uint16_t ServerSocket::initializeServer (const char *peerName){
  _hostAddr.sin_family = AF_INET;
  _hostAddr.sin_port = 0;  //generate a random available port
  _hostAddr.sin_addr.s_addr = INADDR_ANY; //inet_addr(peerName); //receive messages from this address only
  bzero(&_hostAddr.sin_zero, 8);
  if(bind(_socketFd, (sockaddr *) &_hostAddr, sizeof(_hostAddr)) == -1)
    throw "Error binding client handler socket.\n";

  if(getsockname(_socketFd, (sockaddr *) &_hostAddr, &_sinSize) == -1)
    throw "Failed to get client handler socket.";

  return ntohs(_hostAddr.sin_port); //will be sent to the client to start communication at this port
}


char *ServerSocket::getPeerName() const {
  return (char *)inet_ntoa(_peerAddr.sin_addr);
}

int ServerSocket::getPortNumber() const {
  return _peerAddr.sin_port;
}

sockaddr_in ServerSocket::getClientAddress() const {
  return _peerAddr;
}

void ServerSocket::setClientAddress(sockaddr_in client) {
  _peerAddr = client;
}

ServerSocket::~ServerSocket (){

}
