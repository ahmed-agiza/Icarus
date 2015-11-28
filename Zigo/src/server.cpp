#include "server.h"

Server::Server(uint16_t listenPort):_listenPort(listenPort), _terminated(false), _jobCount(0) {
  _jobsPool.initialize(Settings::getInstance().getPoolSize(), true);


  _serverSocket = new UDPSocket;
  _serverSocket->initialize(listenPort);

  if (pthread_mutex_init(&_terminationLock, NULL) != 0)
    throw MutexInitializationException();

  _serverSocket->setMutex(&_terminationLock);
}

void Server::listen() {
  Message request;
  uint32_t serverReplyTo = Settings::getInstance().getServerReplyTimeout();
  while(1){
    printf("Listening..\n");
    while(1) {
      try {
        if(_terminated) {
          break;
        }
        request = _getMessageTimeout(serverReplyTo, 0);
        if (request.getType() != Connect) {
          char invalidRequestMessage[LOG_MESSAGE_LENGTH];
          sprintf(invalidRequestMessage, "Invalid request %s", request.getBody());
          Logger::error(invalidRequestMessage);
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

    char newRequestMessage[LOG_MESSAGE_LENGTH];
    sprintf(newRequestMessage, "Request from %s(%d): %s", _serverSocket->getPeerName(), _serverSocket->getPortNumber(), request.getBody());
    Logger::info(newRequestMessage);
    fflush(stdout);
    serveRequest(request);
  }
  Logger::info("Server terminated!");
}

void Server::serveRequest(Message  &request) {

  char portReply[32];

  char *clientAddrName = (char *)inet_ntoa(_serverSocket->getPeerAddress().sin_addr);
  (void)clientAddrName;

  char *connectionStr = new char[strlen(request.getBody()) + 1];
  strcpy(connectionStr, request.getBody());
  int checkPort = _getClientPort(connectionStr);

  if (checkPort > -1) {
    char portExistsMessage[LOG_MESSAGE_LENGTH];
    sprintf(portExistsMessage, "Connection %s already exists on port %d", connectionStr, checkPort);
    Logger::warn(portExistsMessage);
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

    Job *job = dynamic_cast<Job *>(_jobsPool.acquire());
    ClientNode *client = _addClient(connectionStr, clientPort, job);
    client->setSocket(handlerSocket);
    job->setClient(client);
    job->setSharedData((bool *)&_terminated);

    if(job->start()) {
      printf("Serving client..\n");
    } else {
      Logger::error("Failed to create the server thread.");
      _removeClient(connectionStr);
    }
  }

  delete connectionStr;


}

size_t Server::getJobCount() const {
  return _jobCount;
}

ClientNode *Server::_addClient(char *id, int port, Job *job) {
  if(_clients.find(id) != _clients.end())
    return _clients[id];

  ClientNode *client = new ClientNode(id);
  client->setPort(port);
  client->setJob(job);

  _clients[id] = client;

  return client;
}

int Server::_getClientPort(char *id) {
  if (_clients.find(id) == _clients.end())
    return -1;
  return _clients[id]->getPort();
}

ClientNode *Server::_getClient(char *id) {
  return _clients[id];
}

int Server::_removeClient(char *id) {
  if (_clients.find(id) == _clients.end())
    return -1;

  if(_clients[id])
    delete _clients[id];
  _clients.erase(id);

  return 0;
}

Message Server::_getMessageTimeout(time_t seconds, suseconds_t mseconds) {
  return _serverSocket->recvMessageTimeout(seconds, mseconds);
}

Message Server::_getMessage() {
  return _serverSocket->getMessage();
}

ssize_t Server::_sendMessage(Message message){
  return _serverSocket->sendMessage(message);
}

void Server::_sendReply() {

}

Server::~Server() {
  for (std::map<char *, ClientNode *, StringCompare>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
    ClientNode *client = it->second;
    Job *clientJob = client->getJob();
    if (clientJob) {
      clientJob->wait();
      delete clientJob;
    }
    _removeClient(it->first);
  }
  pthread_mutex_destroy(&_terminationLock);
}
