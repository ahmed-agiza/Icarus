#include "server.h"

Server::Server(uint16_t listenPort):_listenPort(listenPort), _terminated(false), _clientHead(0), _clientTail(0) {
  memset((void *)&_clientsTable, 0, sizeof(_clientsTable));
  hcreate_r(30, &_clientsTable);
  _serverSocket = new ServerSocket;
  _serverSocket->initializeServer(listenPort);
  _serverSocket->setRecvTimeout(2, 0);
  if (pthread_mutex_init(&_terminationLock, NULL) != 0)
    throw "Mutex init failed!\n";
  _serverSocket->setMutex(&_terminationLock);

}

void *requestHandler(void *connection) {
  Server::ClientConnection *clientConnection = (Server::ClientConnection *) connection;
  sockaddr_in clientAddr = clientConnection->getClienAddr();
  ServerSocket *handlerSocket = clientConnection->getSocket();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  printf("Serving client %s from %lu\n", clientAddrName, currentId);

  handlerSocket->setRecvTimeout(2, 0);


  char reply[2048];
  char ack[32];
  char terminationString[] = "q";
  char ackSuccess[] = "1";
  bool *terminated = (bool*) clientConnection->getSharedData();
  while(1){
    Message request;
    while(1) {
      try {
        if(*terminated) {
          break;
        }
        request = handlerSocket->getMessage();
      } catch (const char *timeout) {
        continue;
      }
      break;
    }
    
    if(*terminated){
      printf("Closing %lu because of server termination..\n", currentId);
      break;
    }

    printf("Request from %s(%d): %s\n", handlerSocket->getPeerName(), handlerSocket->getPortNumber(), request.getBody());
    fflush(stdout);

    if(request.getType() == Unknown){
      fprintf(stderr, "Unkown message type.\n");
      continue;
    }

    if(strcmp(request.getBody(), terminationString) == 0){
      handlerSocket->lock();
      *terminated = true;
      break;
    }

    if(*terminated) {
      break;
    }

    int ackLen = sprintf(ack, "%zd", request.getMessagSize());
    ssize_t sentAck = handlerSocket->sendRaw(ack, ackLen);

    const char *ackBack = handlerSocket->getRawMessage();
    if(strcmp(ackBack, ackSuccess) != 0){
        fprintf(stderr, "Failed acknowledgment.\n");
        delete ackBack;
        continue;
    }
    delete ackBack;

    int replySize = sprintf(reply, "You sent: %s", request.getBody());
    Message replyMessage(Reply, strlen(reply), reply);
    const char *replyBytes = replyMessage.getBytes();
    handlerSocket->sendRaw(replyBytes, strlen(replyBytes));
    delete replyBytes;

  }


  printf("Done serving %s(%d)\n", handlerSocket->getPeerName(), handlerSocket->getPortNumber());


  handlerSocket->unlock();
  delete clientConnection;

  pthread_exit(0);
}

int Server::addClient(char *addr, int port, pthread_t *thread) {
  int currentPort = getClientPort(addr);
  if (currentPort > -1)
    return currentPort;

  ENTRY clientEntry, *temp;
  clientEntry.key = new char[64];
  strcpy(clientEntry.key, addr);
  ClientNode* newClient;

  if(!_clientHead){
    newClient = _clientHead = new ClientNode(addr);
    newClient->setPort(port);
    newClient->setThread(thread);
    newClient->setNodeKey(clientEntry.key);
    _clientTail = _clientHead;
  } else {
    newClient = _clientTail->addNext(new ClientNode(addr));
    newClient->setPort(port);
    newClient->setThread(thread);
    newClient->setNodeKey(clientEntry.key);
    _clientTail = newClient;
  }

  clientEntry.data = (void*)newClient;
  hsearch_r(clientEntry, ENTER, &temp, &_clientsTable);
  return port;
}

int Server::getClientPort(char *addr) {
  ENTRY clientQuery, *ret;
  clientQuery.key = addr;
  int found = hsearch_r(clientQuery, FIND, &ret, &_clientsTable);
  if (!found || !ret)
    return -1;
  ClientNode *_client = (ClientNode *) ret->data;
  if(_client)
    return _client->getPort();
  return -1;
}

int Server::removeClient(char *addr) {
  ENTRY clientQuery, *ret;
  clientQuery.key = addr;
  clientQuery.data = 0;
  int found = hsearch_r(clientQuery, FIND, &ret, &_clientsTable);
  if (!found || !ret)
    return -1;

  ClientNode *_client = (ClientNode *) ret->data;

  ret->data = (void *) NULL;

  if(_client->_prev)
    _client->_prev->_next = _client->_next;
  if(_client->_next)
    _client->_next->_prev = _client->_prev;
  if(_clientTail == _client)
    _clientTail = _client->_prev;
  if(_clientHead == _client)
    _clientHead = _client->_next;
  delete _client;
  return 0;
}


void Server::listen() {
  const char *request;
  while(1){
    printf("Listening..\n");
    while(1) {
      try {
        _serverSocket->lock();
        if(_terminated) {
          break;
        }
        _serverSocket->unlock();
        request = _getRawRequest();
      } catch (const char *timeout) {
        continue;
      }
      break;
    }

    if(_terminated) {
      _serverSocket->unlock();
      break;
    }

    printf("Request from %s(%d): %s\n", _serverSocket->getPeerName(), _serverSocket->getPortNumber(), request);
    fflush(stdout);
    serveRequest(request);
  }
  printf("Server terminated!\n");
}

void Server::serveRequest(const char *request) {

  char portReply[32];

  char *clientAddrName = (char *)inet_ntoa(_serverSocket->getClientAddress().sin_addr);

  char *connectionStr = new char[strlen(request) + 1];
  strcpy(connectionStr, request);
  int checkPort = getClientPort(connectionStr);

  if (checkPort > -1) {
    printf("Connection %s already exists on port %d\n", connectionStr, checkPort);
    sprintf(portReply, "%u", checkPort);
    _serverSocket->sendRaw(portReply, strlen(portReply));
  } else {
    ServerSocket *handlerSocket = new ServerSocket();
    handlerSocket->setClientAddress(_serverSocket->getClientAddress());
    handlerSocket->setMutex(&_terminationLock);
    uint16_t clientPort = handlerSocket->initializeServer(clientAddrName);
    sprintf(portReply, "%u", clientPort);
    _serverSocket->sendRaw(portReply, strlen(portReply));

    ClientConnection *connection = new ClientConnection(handlerSocket);
    connection->setSharedData((bool *)&_terminated);

    pthread_t *workerThread = new pthread_t;
    int threadRC = pthread_create(workerThread, 0, requestHandler, (void *) connection);
    if(threadRC){
      fprintf(stderr, "Failed to create the server thread.\n");
    } else {
      addClient(connectionStr, clientPort, workerThread);
      printf("Serving client..\n");
    }
  }

  delete connectionStr;


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
  hdestroy_r(&_clientsTable);
  while(_clientHead){
    ClientNode *temp = _clientHead;
    _clientHead = _clientHead->_next;
    pthread_t *clientThread = temp->getThread();
    if(clientThread) {
      pthread_join(*clientThread, NULL);
      delete clientThread;
    }
    delete temp;
  }
  pthread_mutex_destroy(&_terminationLock);
}
