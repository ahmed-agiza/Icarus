#include "job.h"

Job::Job():Thread(), _client(0){

}

Job::Job(ClientNode *client):Thread(), _client(client){

}

Job::Job(const Job &other):Thread(other), _client(new ClientNode(*other._client)){
  printf("Job(const Job &other)\n");
}


ClientNode *Job::getClient() const {
  return _client;
}
void Job::setClient(ClientNode *client) {
  _client = client;
}


void Job::run() {
  UDPSocket *handlerSocket = _client->getSocket();
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

    if (request.isFileOperation()){
      printf("Type: %d\n", request.getType());
      if (request.getType() == Open) {
        File *file = File::open(request.getBody(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        int fd = file->getFd();
        if (fd < 0)
          throw FileOpenException();
        _client->addFile(fd, file);
        char replyMessage[32];
        sprintf(replyMessage, "%d", fd);
        Message fdReply(Reply, replyMessage);
        handlerSocket->sendMessage(fdReply);

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
        printf("Open succeeded!\n");
      } else if (request.getType() == Read) {
        int fd;
        size_t readSize;
        if(sscanf(request.getBody(), "%d\n%zd", &fd, &readSize) != 2 || readSize > 55000) {
          char invalidRequestMessage[LOG_MESSAGE_LENGTH];
          sprintf(invalidRequestMessage, "Invalid request: %s", request.getBytes());
          Logger::error(invalidRequestMessage);
          continue;
        }
        File *file = _client->getFile(fd);
        printf("Reading(%d): %zd\n", fd, readSize);
        char fileReadBuffer[50000];
        memset(fileReadBuffer, 0, 50000);
        file->read(fileReadBuffer, readSize);
        if (file->isEOF()) {
          printf("EOF!\n");
          Message eofMessage(Eof, "0");
          handlerSocket->sendMessage(eofMessage);
        } else {
          printf("Read: %s\n", fileReadBuffer);
          Message readMessage(Reply, fileReadBuffer);
          handlerSocket->sendMessage(readMessage);
        }

        Message readCharsMessage = handlerSocket->recvMessage();
        size_t readChars = atol(readCharsMessage.getBody());
        printf("Chars read: %zd\n", readChars);
        file->setOffset(readChars);
      } else if (request.getType() == Write) {
        int fd;
        char fileWriteBuffer[50000];
        memset(fileWriteBuffer, 0, 50000);
        size_t writeSize;
        if(sscanf(request.getBody(), "%d\n%zd\n%64999c", &fd, &writeSize, fileWriteBuffer) != 3) {
          char invalidRequestMessage[LOG_MESSAGE_LENGTH];
          sprintf(invalidRequestMessage, "Invalid request: %s", request.getBytes());
          Logger::error(invalidRequestMessage);
          continue;
        }
        File *file = _client->getFile(fd);
        printf("Writing(%d): %zd\n", fd, writeSize);
        size_t writtenBytes = file->write(fileWriteBuffer, writeSize);
        char writtenStr[32];
        sprintf(writtenStr, "%zd", writtenBytes);
        Message writtenByes(Reply, writtenStr);
        handlerSocket->sendMessage(writtenByes);
        printf("Written!\n");
      } else if (request.getType() == Lseek) {
        int fd;
        off_t offset;
        if(sscanf(request.getBody(), "%d\n%zd", &fd, &offset) != 2) {
          char invalidRequestMessage[LOG_MESSAGE_LENGTH];
          sprintf(invalidRequestMessage, "Invalid request: %s", request.getBytes());
          Logger::error(invalidRequestMessage);
          continue;
        }
        File *file = _client->getFile(fd);
        printf("lseek %d -> %zd..\n", fd, offset);
        fflush(stdout);
        file->setOffset(offset);
        char offsetStr[32];
        sprintf(offsetStr, "%zd", offset);
        Message ackMessage(Reply, offsetStr);
        handlerSocket->sendMessage(ackMessage);
      } else if (request.getType() == Close) {
        int fd;
        if(sscanf(request.getBody(), "%d", &fd) != 1) {
          char invalidRequestMessage[LOG_MESSAGE_LENGTH];
          sprintf(invalidRequestMessage, "Invalid request: %s", request.getBytes());
          Logger::error(invalidRequestMessage);
          continue;
        }
        File *file = _client->getFile(fd);
        printf("Closing %d..\n", fd);
        fflush(stdout);
        int closeRc = file->close();
        char closeRcStr[8];
        sprintf(closeRcStr, "%d", closeRc);
        Message closeRcMessage(Reply, closeRcStr);
        handlerSocket->sendMessage(closeRcMessage);
        printf("Close(%s)\n", closeRcStr);
      }
      continue;
    } else if(request.getType() != Request) {
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
  if(_client->getSocket())
    delete _client->getSocket();;
  _client->setSocket(0);
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



Job::~Job(){

}
