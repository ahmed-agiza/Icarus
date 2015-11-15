#include "server.h"

Server::Server(uint16_t listenPort):_listenPort(listenPort), _terminated(false), _clientHead(0), _clientTail(0), _jobCount(0) {
  memset((void *)&_clientsTable, 0, sizeof(_clientsTable));
  hcreate_r(30, &_clientsTable);
  _serverSocket = new UDPSocket;
  _serverSocket->initialize(listenPort);

  if (pthread_mutex_init(&_terminationLock, NULL) != 0)
  throw MutexInitializationException();

  _serverSocket->setMutex(&_terminationLock);

}



void Server::listen() {
  Message request;
  while(1){
    printf("Listening..\n");
    while(1) {
      try {
        if(_terminated) {
          break;
        }
        request = _getMessageTimeout(SERVER_REPLY_TO, 0);
        if (request.getType() != Connect) {
          fprintf(stderr, "Invalid request %s\n", request.getBody());
          continue;
        }
      } catch (ReceiveTimeoutException &timeout) {
        continue;
      }
      break;
    }

    if(_terminated) {
      break;
    }

    printf("Request from %s(%d): %s\n", _serverSocket->getPeerName(), _serverSocket->getPortNumber(), request.getBody());
    fflush(stdout);
    serveRequest(request);
  }
  printf("Server terminated!\n");
}

void Server::serveRequest(Message  &request) {

  char portReply[32];

  char *clientAddrName = (char *)inet_ntoa(_serverSocket->getPeerAddress().sin_addr);

  char *connectionStr = new char[strlen(request.getBody()) + 1];
  strcpy(connectionStr, request.getBody());
  int checkPort = getClientPort(connectionStr);

  if (checkPort > -1) {
    printf("Connection %s already exists on port %d\n", connectionStr, checkPort);
    sprintf(portReply, "%u", checkPort);
    Message portReplyMessage(Accept, portReply);
    _sendMessage(portReplyMessage);
  } else {
    UDPSocket *handlerSocket = new UDPSocket;
    handlerSocket->setPeerAddress(_serverSocket->getPeerAddress());
    handlerSocket->setMutex(&_terminationLock);
    uint16_t clientPort = handlerSocket->initialize(0);
    sprintf(portReply, "%u", clientPort);
    Message portReplyMessage(Accept, portReply);
    _sendMessage(portReplyMessage);

    Job *job = new Job(handlerSocket);
    job->setSharedData((bool *)&_terminated);

    if(job->start()) {
      jobs[_jobCount++] = job;
      addClient(connectionStr, clientPort, job);
      printf("Serving client..\n");
    } else {
      fprintf(stderr, "Failed to create the server thread.\n");
    }
  }

  delete connectionStr;


}

size_t Server::getJobCount() const {
  return _jobCount;
}

int Server::addClient(char *addr, int port, Job *job) {
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
    newClient->setJob(job);
    newClient->setNodeKey(clientEntry.key);
    _clientTail = _clientHead;
  } else {
    newClient = _clientTail->addNext(new ClientNode(addr));
    newClient->setPort(port);
    newClient->setJob(job);
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

Message Server::_getMessageTimeout(time_t seconds, suseconds_t mseconds) {
  return _serverSocket->recvMessageTimeout(seconds, mseconds);
}

Message Server::_getMessage() {
  return _serverSocket->getMessage();
}

ssize_t Server::_sendMessage(Message message){
  _serverSocket->sendMessage(message);
}

void Server::_sendReply() {

}

Server::~Server() {
  hdestroy_r(&_clientsTable);
  while(_clientHead){
    ClientNode *temp = _clientHead;
    _clientHead = _clientHead->_next;
    Job *clientJob = temp->getJob();
    if(clientJob) {
      clientJob->wait();
      delete clientJob;
    }
    delete temp;
  }
  pthread_mutex_destroy(&_terminationLock);
}
