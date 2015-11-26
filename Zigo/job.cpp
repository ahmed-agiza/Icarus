#include "job.h"

Job::Job(UDPSocket *handlerSocket):Thread(), _socket(handlerSocket), _clientAddr(handlerSocket->getPeerAddress()){

}

Job::Job(const Job &other):Thread(other), _socket(new UDPSocket(*other._socket)), _clientAddr(_socket->getPeerAddress()){
  printf("Job(const Job &other)\n");
}

void Job::setSocket(UDPSocket *socket){
  _socket = socket;
  _clientAddr = _socket->getPeerAddress();
}

UDPSocket *Job::getSocket() const {
  return _socket;
}

void Job::run() {
  UDPSocket *handlerSocket = getSocket();
  sockaddr_in clientAddr = handlerSocket->getPeerAddress();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  char servingMessage[50];
  sprintf(servingMessage, "Serving client %s from %lu", clientAddrName, currentId);
  Logger::info(servingMessage);
  //handlerSocket->setRecvTimeout(2, 0);
  char reply[2048];
  char ack[32];
  bool *terminated = (bool*) getSharedData();
  while(1){
    Message request;
    printf("Waiting..\n");
    while(1) {
      try {
        if(*terminated || _terminationRequest()) {
          break;
        }
        request = handlerSocket->recvMessageTimeout(2, 0);
      } catch (ReceiveTimeoutException &timeout) {
        continue;
      }
      break;
    }

    if(*terminated || _terminationRequest()) {
      char serverTerminationMessage[LOG_MESSAGE_LENGTH];
      sprintf(serverTerminationMessage, "Closing %lu because of server termination..", currentId);
      Logger::info(serverTerminationMessage);
      break;
    }

    char newConnectionMessage[LOG_MESSAGE_LENGTH];
    sprintf(newConnectionMessage, "Request from %s(%d): %s", handlerSocket->getPeerName(), handlerSocket->getPortNumber(), request.getBody());
    Logger::info(newConnectionMessage);
    fflush(stdout);

    if(request.getType() != Request) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request type: %s", request.getBytes());
      Logger::error(invalidRequestMessage);
      continue;
    }

    if(request.isTerminationMessage()) {
      handlerSocket->lock();
      *terminated = true;
      break;
    }

    if(*terminated) {
      break;
    }

    int ackLen = sprintf(ack, "%zd", request.getMessagSize());
    (void)ackLen;

    Message ackReply(Acknowledge, ack);
    ssize_t sentAck = handlerSocket->sendMessage(ackReply);
    (void)sentAck;

    Message ackBack = handlerSocket->recvMessage();

    if (ackBack.getType() != Acknowledge) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid message type: %s", ackBack.getBytes());
      Logger::error(invalidReplyMessage);
      continue;
    }
    if(!ackBack.isAcknowledgeSuccess()){
      char misacknowledgmentMessage[LOG_MESSAGE_LENGTH];
      sprintf(misacknowledgmentMessage, "Failed acknowledgment %s.", ackBack.getBytes());
      Logger::error(misacknowledgmentMessage);
      continue;
    }

    int replySize = sprintf(reply, "You sent: %s", request.getBody());
    (void)replySize;

    Message replyMessage(Reply, reply);
    handlerSocket->sendMessage(replyMessage);
  }

  char servingDoneMessage[LOG_MESSAGE_LENGTH];
  sprintf(servingDoneMessage, "Done serving %s(%d)", handlerSocket->getPeerName(), handlerSocket->getPortNumber());
  Logger::info(servingDoneMessage);

  handlerSocket->unlock();
}

bool Job::reset() {
  stop();
  if(_socket)
    delete _socket;
  _socket = 0;
  return true;
}

void Job::stop() {
  Thread::stop();
}

void Job::setSharedData(void *ptr) {
  _shared = ptr;
}

void *Job::getSharedData() const {
  return _shared;
}


void Job::setClientAddr (sockaddr_in clientAddr) {
  _clientAddr = clientAddr;
}

sockaddr_in Job::getClienAddr() const {
  return _clientAddr;
}

Job::~Job(){
  if(_socket){
    delete _socket;
  }
}
