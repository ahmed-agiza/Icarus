#include "server.h"


void *requestHandler(void *connection) {
  Server::ClientConnection *clientConnection = (Server::ClientConnection *) connection;
  sockaddr_in clientAddr = clientConnection->getClienAddr();
  ServerSocket *handlerSocket = clientConnection->getSocket();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  printf("Serving client %s from %lu\n", clientAddrName, currentId);


  char reply[2048];
  char ack[32];
  char terminationString[] = "q";

  while(1){
    printf("Listening for client %s..\n", clientAddrName);
    Message request = handlerSocket->getMessage();
    printf("Request from %s(%d): %s\n", handlerSocket->getPeerName(), handlerSocket->getPortNumber(), request.getBody());
    fflush(stdout);
    int ackLen = sprintf(ack, "%zd", request.getMessagSize());
    ssize_t sentAck = handlerSocket->sendRaw(ack, ackLen);
    int replySize = sprintf(reply, "You sent: %s", request.getBody());
    Message replyMessage(Reply, strlen(reply), reply);
    const char *replyBytes = replyMessage.getBytes();
    handlerSocket->sendRaw(replyBytes, strlen(replyBytes));
    delete replyBytes;
    if(strcmp(request.getBody(), terminationString) == 0){
      break;
    }
  }

  printf("Done serving %s\n", clientAddrName);

  delete clientConnection;
  pthread_exit(0);
}

Server::Server(uint16_t listenPort):_listenPort(listenPort) {
  _serverSocket = new ServerSocket();
  _serverSocket->initializeServer(listenPort);
}

void Server::listen() {
  while(1){
    printf("Listening..\n");
    const char *request = _getRawRequest();
    printf("Request from %s(%d): %s\n", _serverSocket->getPeerName(), _serverSocket->getPortNumber(), request);
    fflush(stdout);
    serveRequest(request);
  }
}

void Server::serveRequest(const char *request) {
  char *clientAddrName = (char *)inet_ntoa(_serverSocket->getClientAddress().sin_addr);

  ServerSocket *handlerSocket = new ServerSocket();
  handlerSocket->setClientAddress(_serverSocket->getClientAddress());

  uint16_t clientPort = handlerSocket->initializeServer(clientAddrName);

  char portReply[32];
  sprintf(portReply, "%u", clientPort);
  printf("%s\n", portReply);
  _serverSocket->sendRaw(portReply, strlen(portReply));

  ClientConnection *connection = new ClientConnection(handlerSocket);

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

const char *Server::_getRawRequest() {
  return _serverSocket->getRawMessage();
}

Message Server::_getMessage() {
  return _serverSocket->getMessage();
}

void Server::_sendReply() {

}

Server::~Server() {

}
