#include "seeder_job.h"

SeederJob::SeederJob():Thread(), _client(0){

}

SeederJob::SeederJob(SeederNode *client):Thread(), _client(client){

}

SeederJob::SeederJob(const SeederJob &other):Thread(other), _client(new SeederNode(*other._client)){
  printf("SeederJob(const SeederJob &other)\n");
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
  (*clients)[(char *)_client->getClientId()] = _client;
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
        request = handlerSocket->recvMessageTimeout(20, 0);
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

    /*char newConnectionMessage[LOG_MESSAGE_LENGTH];
    sprintf(newConnectionMessage, "Ping from %s(%d): %s", handlerSocket->getPeerName(), handlerSocket->getPortNumber(), request.getBody());
    Logger::info(newConnectionMessage);
    fflush(stdout);*/
    if (request.getType() == Query) {
      char result[5000];
      lock();
      for (std::map<char *, SeederNode *, StringCompare>::iterator it = clients->begin(); it != clients->end(); ++it) {
        SeederNode *client = it->second;
        strcat(result, client->getClientId());
        strcat(result, ";");
      }
      unlock();
      Message resultMessage(Reply, result);
      ssize_t sentResult = handlerSocket->sendMessage(resultMessage);
      (void) sentResult;
      continue;
    } else if (request.getType() != Ping) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request type: %s", request.getBytes());
      Logger::error(invalidRequestMessage);
      continue;
    }

    //int ping = sprintf(ack, "%zd", request.getMessagSize());
    //(void)ping;

    Message pingReply(Pong, "1");
    ssize_t sentPong = handlerSocket->sendMessage(pingReply);
    (void)sentPong;
  }
  printf("Client disconneced!\n");
  lock();
  clients->erase((char *)_client->getClientId());
  unlock();

  char servingDoneMessage[LOG_MESSAGE_LENGTH];
  sprintf(servingDoneMessage, "Done serving %s(%d)", handlerSocket->getPeerName(), handlerSocket->getPortNumber());
  Logger::info(servingDoneMessage);
}

bool SeederJob::reset() {
  stop();
  if(_client->getSocket())
    delete _client->getSocket();;
  _client->setSocket(0);
  return true;
}

void SeederJob::stop() {
  Thread::stop();
}

void SeederJob::setSharedData(void *ptr) {
  _shared = ptr;
}

void *SeederJob::getSharedData() const {
  return _shared;
}

SeederJob::~SeederJob(){

}
