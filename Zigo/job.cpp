#include "job.h"

Job::Job(UDPSocket *handlerSocket):Thread(), _socket(handlerSocket), _clientAddr(handlerSocket->getPeerAddress()){

}

UDPSocket *Job::getSocket() const {
  return _socket;
}

void Job::run() {
  UDPSocket *handlerSocket = getSocket();
  sockaddr_in clientAddr = handlerSocket->getPeerAddress();
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
    printf("Waiting..\n");
    while(1) {
      try {
        if(*terminated) {
          break;
        }
        request = handlerSocket->recvMessageTimeout(2, 0);
      } catch (ReceiveTimeoutException &timeout) {
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

    if(request.getType() != Request){
      fprintf(stderr, "Invalid message type...\n\n\n%s", request.getBytes());
      continue;
    }

    if(request.isTerminationMessage()){
      handlerSocket->lock();
      *terminated = true;
      break;
    }

    if(*terminated) {
      break;
    }

    int ackLen = sprintf(ack, "%zd", request.getMessagSize());

    Message ackReply(Acknowledge, ack);
    ssize_t sentAck = handlerSocket->sendMessage(ackReply);

    Message ackBack = handlerSocket->recvMessage();

    if (ackBack.getType() != Acknowledge) {
      fprintf(stderr, "Invalid message type--.\n%s", ackBack.getBody());
      continue;
    }
    if(!ackBack.isAcknowledgeSuccess()){
        fprintf(stderr, "Failed acknowledgment %s.\n", ackBack.getBytes());
        continue;
    }

    int replySize = sprintf(reply, "You sent: %s", request.getBody());
    Message replyMessage(Reply, reply);
    handlerSocket->sendMessage(replyMessage);
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
