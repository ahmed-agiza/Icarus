#include "job.h"

Job::Job(ServerSocket *handlerSocket):Thread(), _socket(handlerSocket), _clientAddr(handlerSocket->getClientAddress()){

}

ServerSocket *Job::getSocket() const {
  return _socket;
}

void Job::run() {
  sockaddr_in clientAddr = getClienAddr();
  ServerSocket *handlerSocket = getSocket();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  printf("Serving client %s from %lu\n", clientAddrName, currentId);

  //handlerSocket->setRecvTimeout(2, 0);
  char reply[2048];
  char ack[32];
  char terminationString[] = "q";
  char ackSuccess[] = "1";
  bool *terminated = (bool*) getSharedData();
  while(1){
    Message request;
    while(1) {
      try {
        if(*terminated) {
          break;
        }
        request = handlerSocket->getMessageTimeout(2, 0);
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
