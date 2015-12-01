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

  while(1){
    Message request;
    while(1) {
      try {
        if(_terminationRequest()) {
          break;
        }
        request = handlerSocket->recvMessageTimeout(10, 0);
      } catch (ReceiveTimeoutException &timeout) {
        continue;
      }
      break;
    }

    if(_terminationRequest()) {
      char serverTerminationMessage[LOG_MESSAGE_LENGTH];
      sprintf(serverTerminationMessage, "Closing %lu because of thread termination..", currentId);
      Logger::info(serverTerminationMessage);
      break;
    }

    char newConnectionMessage[LOG_MESSAGE_LENGTH];
    sprintf(newConnectionMessage, "Ping from %s(%d): %s", handlerSocket->getPeerName(), handlerSocket->getPortNumber(), request.getBody());
    Logger::info(newConnectionMessage);
    fflush(stdout);

    if(request.getType() != Ping) {
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
