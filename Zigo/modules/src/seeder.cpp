#include "seeder.h"

Seeder::Seeder(uint16_t listenPort):_listenPort(listenPort), _jobCount(0) {
  _jobsPool.initialize(Settings::getInstance().getPoolSize(), true);

  _seederSocket = new UDPSocket;
  _seederSocket->initialize(listenPort);

  if (pthread_mutex_init(&_mapLock, NULL) != 0)
    throw MutexInitializationException();

}

void Seeder::listen() {
  Message request;
  uint32_t seederReplyTo = Settings::getInstance().getServerReplyTimeout();

  while(1) {
    printf("Listening..\n");
    while(1) {
      try {
        request = _getMessageTimeout(seederReplyTo, 0);

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

    char newRequestMessage[LOG_MESSAGE_LENGTH];
    //sprintf(newRequestMessage, "Request from %s(%d): %s", _seederSocket->getPeerName(), _seederSocket->getPortNumber(), request.getBody());
    sprintf(newRequestMessage, "Request from %s(%d)", _seederSocket->getPeerName(), _seederSocket->getPortNumber());
    Logger::info(newRequestMessage);
    fflush(stdout);
    serveRequest(request);
  }

  Logger::info("Seeder terminated!");
}

void Seeder::serveRequest(Message  &request) {
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
    Message portReplyMessage(Accept, portReply, SEEDER_ID, DEFAULT_MESSAGE_ID);
    _sendMessage(portReplyMessage);
  } else {
    UDPSocket *handlerSocket = new UDPSocket;
    handlerSocket->setPeerAddress(_seederSocket->getPeerAddress());
    handlerSocket->setMutex(&_mapLock);
    uint16_t clientPort = handlerSocket->initialize(0);
    sprintf(portReply, "%u", clientPort);
    Message portReplyMessage(Accept, portReply, SEEDER_ID, DEFAULT_MESSAGE_ID);
    _sendMessage(portReplyMessage);

    SeederJob *job = dynamic_cast<SeederJob *>(_jobsPool.acquire());
    SeederNode *client = _addClient(rsa, clientPort, job);
    client->setSocket(handlerSocket);
    client->setUsername(username);
    client->setServerPort(serverPort);

    job->setClient(client);
    job->setId(SEEDER_ID);
    job->setSharedData((SeedersMap *)&_clients);
    job->addDoneCallback(_threadDoneWrapper, this);

    if(job->start()) {
      printf("Serving client..\n");
    } else {
      Logger::error("Failed to create the Seeder thread.");
      _removeClient(connectionStr);
    }
  }

  delete connectionStr;
}

size_t Seeder::getJobCount() const {
  return _jobCount;
}

SeederNode *Seeder::_addClient(char *key, int port, SeederJob *job) {
  if(_clients.find(key) != _clients.end())
    return _clients[key];

  SeederNode *client = new SeederNode(key);

  client->setPort(port);
  client->setJob(job);

  return client;
}

int Seeder::_getClientPort(char *id) {
  if (_clients.find(id) == _clients.end())
    return -1;
  return _clients[id]->getPort();
}

SeederNode *Seeder::_getClient(char *id) {
  return _clients[id];
}

int Seeder::_removeClient(char *id) {
  if (_clients.find(id) == _clients.end())
    return -1;
  printf("Erasing..\n");
  if(_clients[id])
    delete _clients[id];
  _clients.erase(id);
  printf("Erased!\n");

  return 0;
}

Message Seeder::_getMessageTimeout(time_t seconds, suseconds_t mseconds) {
  return _seederSocket->recvMessageTimeout(seconds, mseconds);
}

Message Seeder::_getMessage() {
  return _seederSocket->getMessage();
}

ssize_t Seeder::_sendMessage(Message message){
  return _seederSocket->sendMessage(message);
}

void Seeder::_sendReply() {

}

void *Seeder::_threadDoneWrapper(Thread *thread, void* parent) {
  SeederJob * job = static_cast<SeederJob *>(thread);
  Seeder *seeder = static_cast<Seeder *>(parent);
  seeder->_threadDoneCallback(job);
  return (void *)thread;
}
void Seeder::_threadDoneCallback(SeederJob *job) {
  printf("Releasing!\n");
  _removeClient((char *) job->getClient()->getClientId());
  _jobsPool.release(job);
  printf("Done!\n");
}

Seeder::~Seeder() {
  for (std::map<char *, SeederNode *, StringCompare>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
    SeederNode *client = it->second;
    SeederJob *clientJob = client->getJob();
    if (clientJob) {
      clientJob->wait();
      delete clientJob;
    }
    _removeClient(it->first);
  }
  pthread_mutex_destroy(&_mapLock);
}
