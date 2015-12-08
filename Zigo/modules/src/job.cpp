#include "job.h"

Job::Job():Thread(), _client(0){
  memset(_id, 0, 128);
}

Job::Job(ClientNode *client):Thread(), _client(client){
  memset(_id, 0, 128);
}

Job::Job(const char *id):Thread(), _client(0) {
  strcpy(_id, id);
}

Job::Job(const Job &other):Thread(other), _client(new ClientNode(*other._client)){
  strcpy(_id, other._id);
}


ClientNode *Job::getClient() const {
  return _client;
}
void Job::setClient(ClientNode *client) {
  _client = client;
}

void Job::handleRemoteFile(Message &request) {
  char fileId[PATH_MAX], replyMessage[32];;
  int mode;
  if (request.getType() == Open) {
    char filePath[PATH_MAX];

    if(sscanf(request.getBody(), "%[^;]%*c%d;", fileId, &mode) != 2) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request body\n%s\nin\n%s\n", request.getBody(), request.getBytes());
      Logger::error(invalidRequestMessage);
      throw InvalidReplyException();
    }

    sprintf(filePath, "storage/%s/recv", request.getOwnerId());
    File::createDirIfNotExists(filePath);
    sprintf(filePath, "%s/%s", filePath, fileId);
    if (mode == ReadOnly) {
      if (!File::exists(filePath)) {
        sprintf(replyMessage, "-1;%d;", (int) Noent);
        Message enoentMessage(Reply, replyMessage, _id, fileId);
        _handlerSocket->sendMessage(enoentMessage);
        return;
      }
    }
    if (mode == AttemptCreate) {
      printf("AttemptCreate: %d\n", (int)File::exists(filePath));
      if (File::exists(filePath)) {
        sprintf(replyMessage, "-1;%d;", (int) AlreadyExists);
        Message existsMessage(Reply, replyMessage, _id, fileId);
        _handlerSocket->sendMessage(existsMessage);
        return;
      }
    } else if (File::exists(filePath)) {
      if (File::isLocked(filePath)) {
        sprintf(replyMessage, "-1;%d;", (int) Locked);
        Message lockedMessage(Reply, replyMessage, _id, fileId);
        _handlerSocket->sendMessage(lockedMessage);
        return;
      }
    }
    File *file;
    if (mode == ReadOnly)
      file = File::open(filePath, O_RDONLY);
    else {
      file = File::open(filePath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
      file->lock();
    }
    int fd = file->getFd();
    if (fd < 0)
      throw FileOpenException();
    _client->addFile(fd, file);
    sprintf(replyMessage, "%d;%d;", fd, (int) Opened);
    Message fdReply(Reply, replyMessage, _id, fileId);
    _handlerSocket->sendMessage(fdReply);

    Message ackBack = _handlerSocket->recvMessage();

    if (ackBack.getType() != Acknowledge) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid message type: %s", ackBack.getBytes());
      Logger::error(invalidReplyMessage);
      return;
    }
    if(!ackBack.isAcknowledgeSuccess()){
      char misacknowledgmentMessage[LOG_MESSAGE_LENGTH];
      sprintf(misacknowledgmentMessage, "Failed acknowledgment %s.", ackBack.getBytes());
      Logger::error(misacknowledgmentMessage);
      return;
    }
    printf("Open succeeded!\n");
  } else if (request.getType() == Close) {
    int fd;
    if(sscanf(request.getBody(), "%d", &fd) != 1) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request: %s", request.getBytes());
      Logger::error(invalidRequestMessage);
      return;
    }
    File *file = _client->getFile(fd);
    if(!file) {
      sprintf(replyMessage, "-1;%d;", (int) Noent);
      Message enoentMessage(Reply, replyMessage, _id, fileId);
      _handlerSocket->sendMessage(enoentMessage);
      return;
    }
    printf("Closing %d..\n", fd);
    fflush(stdout);
    int closeRc = file->close();
    char closeRcStr[8];
    if(closeRc != 0)
      sprintf(closeRcStr, "%d;%d;", fd, (int)FailedOperation);
    else
      sprintf(closeRcStr, "%d;%d;", fd, (int)Closed);
    Message closeRcMessage(Reply, closeRcStr, _id, fileId);
    _handlerSocket->sendMessage(closeRcMessage);
    printf("Close(%s)\n", closeRcStr);
  } else if (request.getType() == Write) {
    int fd;
    char fileWriteBuffer[50000];
    char decodedWriteBuffer[50000];
    memset(fileWriteBuffer, 0, 50000);
    size_t decodeLen = 50000;
    size_t writeSize;
    if(sscanf(request.getBody(), "%d\n%zd\n%64999c", &fd, &writeSize, fileWriteBuffer) != 3) {
      char invalidRequestMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidRequestMessage, "Invalid request: %s", request.getBytes());
      Logger::error(invalidRequestMessage);
      return;
    }

    Crypto::base64Decode(fileWriteBuffer, strlen(fileWriteBuffer), (unsigned char *)decodedWriteBuffer, &decodeLen);

    File *file = _client->getFile(fd);

    if(file->isLocked() && !file->isLockOwner()) {
      Logger::warn("Attempting to write on locked file.");
      Message writtenByes(Reply, "0", _id, DEFAULT_MESSAGE_ID);
      return;
    }

    size_t writtenBytes;

    if(request.getEncoding() == RSAEncryption) {
      char decryptedWriteBuffer[50000];
      Crypto::decrypt((char *) getServerRSA(), decryptedWriteBuffer, decodedWriteBuffer);
      writtenBytes = file->write(decodedWriteBuffer, writeSize);
    } else
       writtenBytes = file->write(decodedWriteBuffer, decodeLen);
    char writtenStr[32];
    sprintf(writtenStr, "%zd", writtenBytes);
    Message writtenByes(Reply, writtenStr, _id, DEFAULT_MESSAGE_ID);
    _handlerSocket->sendMessage(writtenByes);

  }else {
    printf("Invalid request!\n");
  }
}


void Job::run() {
  _handlerSocket = _client->getSocket();
  sockaddr_in clientAddr = _handlerSocket->getPeerAddress();
  char *clientAddrName = (char *)inet_ntoa(clientAddr.sin_addr);

  pthread_t currentId = pthread_self();
  char servingMessage[50];
  sprintf(servingMessage, "Serving client %s from %lu", clientAddrName, currentId);
  Logger::info(servingMessage);
  //_handlerSocket->setRecvTimeout(2, 0);
  char reply[2048];
  char ack[32];
  bool terminated = false;

  while(1){
    Message request;
    printf("Waiting..\n");
    try {
      while(1) {
        try {
          if(terminated || _terminationRequest()) {
            break;
          }
          request = _handlerSocket->recvMessageTimeout(2, 0);
        } catch (ReceiveTimeoutException &timeout) {
          continue;
        }
        break;
      }

      if(terminated || _terminationRequest()) {
        char serverTerminationMessage[LOG_MESSAGE_LENGTH];
        sprintf(serverTerminationMessage, "Closing %lu because of server termination..", currentId);
        Logger::info(serverTerminationMessage);
        break;
      }

      char newConnectionMessage[LOG_MESSAGE_LENGTH];
      sprintf(newConnectionMessage, "Request from %s(%d)", _handlerSocket->getPeerName(), _handlerSocket->getPortNumber());
      Logger::info(newConnectionMessage);
      fflush(stdout);

      if (request.isFileOperation()){
        handleRemoteFile(request);
        continue;
      } else if(request.getType() != Request) {
        char invalidRequestMessage[LOG_MESSAGE_LENGTH];
        sprintf(invalidRequestMessage, "Invalid request type: %s", request.getBytes());
        Logger::error(invalidRequestMessage);
        continue;
      }

      if(request.isTerminationMessage()) {
        _handlerSocket->lock();
        terminated = true;
        break;
      }

      if(terminated) {
        break;
      }

      int ackLen = sprintf(ack, "%zd", request.getMessagSize());
      (void)ackLen;

      Message ackReply(Acknowledge, ack, _id, DEFAULT_MESSAGE_ID);
      ssize_t sentAck = _handlerSocket->sendMessage(ackReply);
      (void)sentAck;

      Message ackBack = _handlerSocket->recvMessage();

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

      int replySize;

      if (strcmp(request.getBody(), "rsa") == 0) {
        replySize = sprintf(reply, "%s", getServerRSA());
      } else if (strcmp(request.getBody(), "steg") == 0) {
        replySize = sprintf(reply, "%s", getStegKey());
      } else {
        replySize = sprintf(reply, "You sent: %s", request.getBody());
      }
      (void) replySize;

      Message replyMessage(Reply, reply, _id, DEFAULT_MESSAGE_ID);
      _handlerSocket->sendMessage(replyMessage);
    } catch (NetworkException &e) {
      Logger::error(e.what());
    }

  }


  char servingDoneMessage[LOG_MESSAGE_LENGTH];
  sprintf(servingDoneMessage, "Done serving %s(%d)", _handlerSocket->getPeerName(), _handlerSocket->getPortNumber());
  Logger::info(servingDoneMessage);

  _handlerSocket->unlock();
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


void Job::setId(char *id) {
  strcpy(_id, id);
}

void Job::setServerRSA(char *key) {
  _serverRSA = key;
}

const char *Job::getServerRSA() const {
  return _serverRSA;
}

void Job::setStegKey(char *key) {
  _serverStegKey = key;
}

const char *Job::getStegKey() const {
  return _serverStegKey;
}


Job::~Job(){

}
