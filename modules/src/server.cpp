#include "server.h"
#include <iostream>
using namespace std;
Server::Server(uint16_t listenPort):_listenPort(listenPort), _terminated(false), _jobCount(0) {
  _jobsPool.initialize(Settings::getInstance().getPoolSize(), true);

  if (!File::exists(PUBLIC_KEY_PATH) || !File::exists(PRIVATE_KEY_PATH)) {
    Crypto::generateKeyPair(PRIVATE_KEY_PATH, PUBLIC_KEY_PATH);
  }

  File *publicKeyFile = File::open(PUBLIC_KEY_PATH, O_RDONLY);
  File *privateKeyFile = File::open(PRIVATE_KEY_PATH, O_RDONLY);
  memset(_id, 0, 128);
  memset(_publicRSA, 0, 2048);
  memset(_privateRSA, 0, 2048);
  memset(_stegKey, 0, 2048);
  Crypto::generateRandomString(_stegKey, 128);
  publicKeyFile->read(_publicRSA, 2048);
  privateKeyFile->read(_privateRSA, 2048);
  publicKeyFile->close();
  privateKeyFile->close();
  delete publicKeyFile;
  delete privateKeyFile;

  Crypto::md5Hash(_publicRSA, _id);


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
    fflush(stdout);
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
    //sprintf(newRequestMessage, "Request from %s(%d): %s", _serverSocket->getPeerName(), _serverSocket->getPortNumber(), request.getBody());
    sprintf(newRequestMessage, "Request from %s(%d)", _serverSocket->getPeerName(), _serverSocket->getPortNumber());
    Logger::info(newRequestMessage);
    fflush(stdout);
    serveRequest(request);
  }
  Logger::info("Server terminated!");
}

const char *Server::getStegKey() {
  return _stegKey;
}

const char *Server::getPublicRSA() {
  return _publicRSA;
}
const char *Server::getPrivateRSA() {
  return _privateRSA;
}

void Server::serveRequest(Message  &request) {
  char portReply[32], username[128], rsa[2048], verificationToken[65], encryptedToken[256], serverPortStr[64];
  uint16_t serverPort;
  uint32_t seederReplyTo = Settings::getInstance().getServerReplyTimeout();

  char *connectionStr = new char[strlen(request.getBody()) + 1];
  strcpy(connectionStr, request.getBody());
  if (sscanf(connectionStr, "%[^;]%*c%[^;]%*c%2048c", username, serverPortStr, rsa) != 3) {
    char invalidConnectionString[LOG_MESSAGE_LENGTH];
    sprintf(invalidConnectionString, "Invalid connection string %s", connectionStr);
    Logger::error(invalidConnectionString);
    delete connectionStr;
    return;
  }

  serverPort = (uint16_t) atoi(serverPortStr);

  Crypto::generateRandomString(verificationToken, 64);

  int encryptionLength = Crypto::encrypt(rsa, verificationToken, encryptedToken);

  Message verificationMessage(Reply, encryptedToken, SEEDER_ID, DEFAULT_MESSAGE_ID, Base64, encryptionLength);
  _sendMessage(verificationMessage);

  try {
    Message verificationReply = _getMessageTimeout(seederReplyTo, 0);

    if (verificationReply.getType() != Verify) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request %s", request.getBody());
      Logger::error(invalidRequestMessage);
      return;
    } else if (strcmp(verificationReply.getBody(), verificationToken) != 0) {
      char invalidTokenMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidTokenMessage, "Invalid token %s != %s", request.getBody(), verificationToken);
      Logger::error(invalidTokenMessage);
      return;
    }
  } catch (ReceiveTimeoutException &timeout) {
    return;
  }


  int checkPort = _getClientPort(connectionStr);

  if (checkPort > -1) {
    char portExistsMessage[LOG_MESSAGE_LENGTH];
    sprintf(portExistsMessage, "Connection %s already exists on port %d", connectionStr, checkPort);
    Logger::warn(portExistsMessage);
    sprintf(portReply, "%u", checkPort);
    Message portReplyMessage(Accept, portReply, _id, DEFAULT_MESSAGE_ID);
    _sendMessage(portReplyMessage);
  } else {

    UDPSocket *handlerSocket = new UDPSocket;
    handlerSocket->setPeerAddress(_serverSocket->getPeerAddress());
    handlerSocket->setMutex(&_terminationLock);

    uint16_t clientPort = handlerSocket->initialize(0);

    sprintf(portReply, "%u", clientPort);

    Message portReplyMessage(Accept, portReply, _id, DEFAULT_MESSAGE_ID);
    _sendMessage(portReplyMessage);


    Job *job = dynamic_cast<Job *>(_jobsPool.acquire());
    job->setId(_id);
    job->addDoneCallback(_threadDoneWrapper, this);
    job->setServerRSA(_publicRSA);
    job->setRSA(_privateRSA);
    job->setStegKey(_stegKey);

    ClientNode *client = _addClient(connectionStr, clientPort, job);
    client->setSocket(handlerSocket);
    client->setUsername(username);
    client->setServerPort(serverPort);



    job->setClient(client);

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

ClientNode *Server::_addClient(char *key, int port, Job *job) {
  char id[128];
  Crypto::md5Hash(key, id);
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

void Server::run() {
  listen();
}

bool Server::reset() {
  stop();
  return true;
}

void Server::stop() {
  Thread::stop();
}

void *Server::_threadDoneWrapper(Thread *thread, void* parent) {
  Job * job = static_cast<Job *>(thread);
  Server *server = static_cast<Server *>(parent);
  server->_threadDoneCallback(job);
  return (void *)thread;
}
void Server::_threadDoneCallback(Job *job) {
  _removeClient((char *)job->getClient()->getClientId());
  _jobsPool.release(job);
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
