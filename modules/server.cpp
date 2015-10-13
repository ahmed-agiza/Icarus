#include "server.h"

Server::Server(uint16_t listenPort):_listenPort(listenPort) {
  _serverSocket = new ServerSocket();
  _serverSocket->initializeServer(listenPort);
}

void Server::listen() {
  while(1){
    printf("Listening..\n");
    char *request = _getRequest();
    printf("%s\n", request);
    fflush(stdout);
    delete request;
  }
}

char *Server::_getRequest() {
  ssize_t recvBytes = _serverSocket->recvRaw();
  if(recvBytes < 0)
    fprintf(stderr, "%s\n", "An error occured while receiving the message.");
  char *recvData = new char[recvBytes];
  strcpy(recvData, _serverSocket->getReadBuff());
  return recvData;
}
void Server::_sendReply() {

}

Server::~Server() {

}
