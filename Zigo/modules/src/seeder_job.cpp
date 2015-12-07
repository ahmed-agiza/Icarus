#include "seeder_job.h"

SeederJob::SeederJob():Thread(), _client(0){
  memset(_id, 0, 128);
}

SeederJob::SeederJob(const char *id):Thread(), _client(0){
  strcpy(_id, id);
}

SeederJob::SeederJob(SeederNode *client):Thread(), _client(client){
  memset(_id, 0, 128);
}

SeederJob::SeederJob(const SeederJob &other):Thread(other), _client(new SeederNode(*other._client)){
  strcpy(_id, other._id);
}

SeederNode *SeederJob::getClient() const {
  return _client;
}
void SeederJob::setClient(SeederNode *client) {
  _client = client;
}

void SeederJob::run() {
  UDPSocket *handlerSocket = _client->getSocket();
  sockaddr_in clientAddr = handlerSocket->getPeerAddress();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  char servingMessage[50];
  sprintf(servingMessage, "Serving client %s from %lu", clientAddrName, currentId);
  Logger::info(servingMessage);

  SeedersMap *clients = static_cast<SeedersMap*>(getSharedData());

  lock();
  printf("Adding user %s\n", _client->getUsername());
  fflush(stdout);
  (*clients)[(char *)_client->getClientId()] = _client;
  printf("Added user %s\n", _client->getUsername());
  fflush(stdout);
  unlock();

  printf("%s\n", _client->getClientId());

  bool clientTerminated = false;

  while(!clientTerminated){
    Message request;
    while(!clientTerminated) {
      try {
        if(_terminationRequest()) {
          break;
        }
        request = handlerSocket->recvMessageTimeout(10, 0);
      } catch (ReceiveTimeoutException &timeout) {
        clientTerminated = true;
      }
      break;
    }

    if(clientTerminated || _terminationRequest()) {
      char serverTerminationMessage[LOG_MESSAGE_LENGTH];
      sprintf(serverTerminationMessage, "Closing %lu because of thread termination..", currentId);
      Logger::info(serverTerminationMessage);
      break;
    }


    if (request.getType() == Query) {
      char result[5000];
      char peer[2048];
      char body[128], type[128];
      int qtype;
      sscanf(request.getBody(), "%[^=]=%s", type, body);
      if (strcmp(type, "username") == 0)
        qtype = 1;
      else if (strcmp(type, "id") == 0)
        qtype = 2;
      else  if (strcmp(request.getBody(), "1") == 0)
        qtype = 3;
      else {
        char invalidRequestMessage[LOG_MESSAGE_LENGTH];
        sprintf(invalidRequestMessage, "Invalid request body: %s", request.getBytes());
        Logger::error(invalidRequestMessage);
        continue;
      }
      lock();
      for (std::map<char *, SeederNode *, StringCompare>::iterator it = clients->begin(); it != clients->end(); ++it) {
        SeederNode *client = it->second;
        if (qtype == 3 || (qtype == 1 && strcmp(body, client->getUsername()) == 0)) {
          client->getPeer(peer);
          strcat(result, peer);
          strcat(result, ";");
        }else if (qtype == 2 && strcmp(client->getClientId(), body) == 0) {
          client->getPeer(peer);
          strcat(result, peer);
          strcat(result, ";");
          break;
        }

      }
      unlock();
      Message resultMessage(Reply, result, _id, DEFAULT_MESSAGE_ID);
      ssize_t sentResult = handlerSocket->sendMessage(resultMessage);
      (void) sentResult;
      continue;
    } else if (request.getType() != Ping) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request type: %s", request.getBytes());
      Logger::error(invalidRequestMessage);
      continue;
    }

    Message pongReply(Pong, "1", _id, DEFAULT_MESSAGE_ID);
    ssize_t sentPong = handlerSocket->sendMessage(pongReply);
    (void)sentPong;
  }
  printf("Client disconneced!\n");
  lock();
  printf("Removing user: %s\n", _client->getUsername());
  char *clientId = (char *) _client->getClientId();
  clients->erase(clientId);
  printf("Removed %s\n", _client->getUsername());
  unlock();

  char servingDoneMessage[LOG_MESSAGE_LENGTH];
  printf("getPeerName() %d\n", (int)(handlerSocket == NULL));
  fflush(stdout);
  sprintf(servingDoneMessage, "Done serving %s(%d)", handlerSocket->getPeerName(), handlerSocket->getPortNumber());
  printf("Done!\n");
  fflush(stdout);
  Logger::info(servingDoneMessage);
}

bool SeederJob::reset() {
  stop();
  if(_client->getSocket()) {
    printf("Delete Socket!\n");
    delete _client->getSocket();
    printf("Deleted!\n");
  }
  _client->setSocket(0);
  return true;
}

void SeederJob::stop() {
  Thread::stop();
}

void SeederJob::setId(char *id) {
  strcpy(_id, id);
}

SeederJob::~SeederJob(){

}
