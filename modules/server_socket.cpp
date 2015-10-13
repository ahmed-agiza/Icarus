#include "server_socket.h"

ServerSocket::ServerSocket ():UDPSocket(){
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
}
ServerSocket::~ServerSocket (){

}
