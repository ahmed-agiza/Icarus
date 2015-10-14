#include "server.h"


void *requestHandler(void *connection) {
  Server::ClientConnection *clientConnection = (Server::ClientConnection *) connection;
  Server *server = clientConnection->getServer();
  sockaddr_in clientAddr = clientConnection->getClienAddr();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  printf("Serving client %s from %lu\n", clientAddrName, currentId);

  ServerSocket *handlerSocket = new ServerSocket();
  uint16_t clientPort = handlerSocket->initializeServer(clientAddrName);
  char portReply[32];
  sprintf(portReply, "%u", clientPort);
  printf("%s\n", portReply);
  server->_serverSocket->sendRaw(portReply, strlen(portReply));

  char reply[2048];
  char ack[32];
  char terminationString[] = "q";

  while(1){
    printf("Listening for client %s..\n", clientAddrName);
    printf("%s(%d)\n", handlerSocket->getPeerName(), handlerSocket->getPortNumber());
    const char *request = handlerSocket->getMessage();
    printf("Request from %s(%d): %s\n", handlerSocket->getPeerName(), handlerSocket->getPortNumber(), request);
    fflush(stdout);
    int ackLen = sprintf(ack, "%d", (int)strlen(request));
    ssize_t sendAck = handlerSocket->sendRaw(ack, ackLen);
    int replySize = sprintf(reply, "You sent: %s", request);
    server->_serverSocket->sendRaw(reply, replySize, clientAddr);
    if(strcmp(request, terminationString) == 0){
      break;
    }
  }

  printf("Done serving %s!", clientAddrName);
  /*server->_acknowledge(clientConnection->getRequest());

  char reply[2048];
  int replySize = sprintf(reply, "You sent: %s", clientConnection->getRequest());
  server->_serverSocket->sendRaw(reply, replySize, clientConnection->getClienAddr());*/
  delete handlerSocket;
  delete clientConnection;

  printf("Done serving..\n");
  pthread_exit(0);
}

Server::Server(uint16_t listenPort):_listenPort(listenPort) {
  _serverSocket = new ServerSocket();
  _serverSocket->initializeServer(listenPort);
}

void Server::listen() {
  while(1){
    printf("Listening..\n");
    const char *request = _getRequest();
    printf("Request from %s(%d): %s\n", _serverSocket->getPeerName(), _serverSocket->getPortNumber(), request);
    fflush(stdout);
    serveRequest(request);
  }
}

void Server::serveRequest(const char *request) {
  ClientConnection *connection = new ClientConnection(this, request, _serverSocket->getClientAddress());

  pthread_t *workerThread = new pthread_t;
  int threadRC = pthread_create(workerThread, 0, requestHandler, (void *) connection);
  if(threadRC){
    fprintf(stderr, "Failed to create the server thread.\n");
  } else {
    printf("Serving client..\n");
  }
}

ssize_t Server::_acknowledge(const char *request) {
    char ack[32];
    int ackLen = sprintf(ack, "%d", (int)strlen(request));
    return _serverSocket->sendRaw(ack, ackLen);
}

bool Server::_acknowledgeAndWait(const char *request) {
    _acknowledge(request);
    return true;
}

const char *Server::_getRequest() {
  return _serverSocket->getMessage();
}
void Server::_sendReply() {

}

Server::~Server() {

}
