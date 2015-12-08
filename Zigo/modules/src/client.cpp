#include "client.h"


Client::Client(const char *username, const char * hostname, uint16_t port, uint16_t serverPort): Thread(), _clientSocket(0), _connectionPort(port), _executed(false), _busy(false), _currentOperation(Idle), _serverPort(serverPort) {
  strcpy(_hostname, hostname);
  strcpy(_username, username);
  memset(_id, 0, 128);

  if (true || !File::exists(PUBLIC_KEY_PATH) || !File::exists(PRIVATE_KEY_PATH)) {
    printf("Generating RSA key pair!\n");
    Crypto::generateKeyPair(PRIVATE_KEY_PATH, PUBLIC_KEY_PATH);
  }

  File *publicKeyFile = File::open(PUBLIC_KEY_PATH, O_RDONLY);
  File *privateKeyFile = File::open(PRIVATE_KEY_PATH, O_RDONLY);
  memset(_id, 0, 128);
  memset(_publicRSA, 0, 2048);
  memset(_privateRSA, 0, 2048);
  publicKeyFile->read(_publicRSA, 2048);
  privateKeyFile->read(_privateRSA, 2048);
  publicKeyFile->close();
  privateKeyFile->close();
  delete publicKeyFile;
  delete privateKeyFile;

  Crypto::md5Hash(_publicRSA, _id);
  printf("Client ID: %s\n", _id);

  if (pthread_mutex_init(&_operationLock, NULL) != 0)
    throw MutexInitializationException();
  if (pthread_mutex_init(&_fetchingCvLock, NULL) != 0)
    throw MutexInitializationException();
  if (pthread_cond_init(&_fetchingCv, NULL))
    throw CVInitializationException();
  setMutex(&_operationLock);
  setCV(&_fetchingCv);



}

int Client::_establishConnection() {
  if (_clientSocket)
    delete _clientSocket;

  _clientSocket = new UDPSocket;
  _clientSocket->initialize(_hostname, _connectionPort);

  char connectionString[2176], verificationStr[2048];
  unsigned char decoded[2048];
  size_t decodedLength = 2048;
  sprintf(connectionString, "%s;%d;%s", _username, (int) _serverPort, _publicRSA);

  try {
    Message connectionMessage = Message(Connect, connectionString, _id, DEFAULT_MESSAGE_ID);

    ssize_t sentBytes = _sendMessage(connectionMessage);

    if(sentBytes < 0)
      return -2;
    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

    printf("Attempting to connect..\n");
    fflush(stdout);
    Message tokenReply = _getReplyTimeout(clientReplyTo, 0);
    if (tokenReply.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", tokenReply.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }

    Crypto::base64Decode((char *)tokenReply.getBody(), strlen(tokenReply.getBody()), decoded, &decodedLength);
    Crypto::decrypt(_privateRSA, (char *)decoded, verificationStr);

    Message verificationMessage(Verify, verificationStr, _id, DEFAULT_MESSAGE_ID);
    _sendMessage(verificationMessage);

    Message portReply = _getReplyTimeout(clientReplyTo, 0);

    if (portReply.getType() != Accept) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", portReply.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }

    _port = (uint16_t)strtoull(portReply.getBody(), NULL, 0);
    char connectionPortMessage[LOG_MESSAGE_LENGTH];
    sprintf(connectionPortMessage, "Connecting on port: %u", _port);
    Logger::info(connectionPortMessage);

    _clientSocket->setPort(_port);
    return 0;
  } catch (ReceiveTimeoutException &e) {
    fprintf(stderr, "--%s\n", e.what());
    return -1;
  }
}

void Client::run() {
  _busy = true;
  if (_establishConnection() < 0) {
    printf("Connection failed!\n");
    return;
  }
  try {
    execute();
  } catch(NetworkException &e) {
    _resultState = Failed;
    Logger::error(e.what());
    resume();
  }
  _busy = false;
}

//start sending messages from client.
int Client::execute() {
  printf("execute()\n");
  _resultState = Fetching;
  ssize_t sentAckBytes;
  int success = 0;
  int retry = -1;
  char ackBack[2];
  bool terminated = false;
  (void) terminated;

  lock();
  Message requestMessage = Message(Request, _queryParam, _id, DEFAULT_MESSAGE_ID); //wrap the text in message form
  unlock();
  uint32_t maxRetry = Settings::getInstance().getRetryTimes();
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

  while(!success && retry < (int)maxRetry) { //try to re-send packet if failed within MAX_RETRY

    ssize_t sentBytes = _sendMessage(requestMessage); //send message to server

    if(sentBytes < 0)
      return -1;

    if(_terminationRequest()){
      break;
    }

    Message ackReply = _getReplyTimeout(clientReplyTo, 0);

    if (ackReply.getType() != Acknowledge) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", ackReply.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }

    uint32_t ackNumber = (uint32_t) strtoull(ackReply.getBody(), NULL, 0);

    success = (ackNumber == sentBytes);


    sprintf(ackBack, "%d", (int) success);
    Message ackBackReply(Acknowledge, ackBack, _id, DEFAULT_MESSAGE_ID);

    sentAckBytes = _sendMessage(ackBackReply);
    (void) sentAckBytes;

    if(!success){ //packet loss
      char misacknowledgmentMessage[LOG_MESSAGE_LENGTH];
      sprintf(misacknowledgmentMessage, "Mismatch between acknowledgment and sent bytes (%d==%d)?.", (int)sentBytes, (int) ackNumber);
      Logger::error(misacknowledgmentMessage);
      retry++;
      if(retry < (int)maxRetry){
        char retryMessage[LOG_MESSAGE_LENGTH];
        sprintf(retryMessage, "Retrying to send the message(%d)..", retry);
        Logger::warn(retryMessage);
      }
    }
  }


  if(!success){
    Logger::error("Failed to deliver your message.");
    return -1;
  }




  Message replyMessage = _getReplyTimeout(clientReplyTo, 0);
  if (replyMessage.getType() != Reply) {
    char invalidReplyMessage[LOG_MESSAGE_LENGTH];
    sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
    Logger::error(invalidReplyMessage);
    throw InvalidReplyException();
  }
  const char *reply = replyMessage.getBody();

  strcpy(_results, reply);
  _resultState = Ready;
  resume();
  return 0;
  /*char tempBuff[2048]; //max char from user input
  char ackBack[2];
  bool success; //success if message was sent wihtout any packet loss
  ssize_t sentBytes;  //number of bytes sent to the server
  ssize_t sentAckBytes;
  int retry;
  File *file;
  Message requestMessage;
  while(1) {
    success = 0;
    retry = -1;
    printf("Enter message to send:\n");
    fgets(tempBuff, sizeof tempBuff, stdin);
    tempBuff[strlen(tempBuff) - 1] = 0;
    if (strcmp(tempBuff, "f") == 0) {
      printf("Enter file operation (o=open, r=read, w=write, l=lseek, c=close): ");
      fgets(tempBuff, sizeof tempBuff, stdin);
      tempBuff[strlen(tempBuff) - 1] = 0;
      if (strcmp(tempBuff, "o") == 0) {
        printf("Enter file name: ");
        fgets(tempBuff, sizeof tempBuff, stdin);
        tempBuff[strlen(tempBuff) - 1] = 0;
        file = File::ropen(_clientSocket, tempBuff, _id);
        int fd = file->getFd();
        printf("File descriptor: %d\n", fd);

        success = 1;

        sprintf(ackBack, "%d", (int) success);
        Message ackBackReply(Acknowledge, ackBack, _id, DEFAULT_MESSAGE_ID);

        sentAckBytes = _sendMessage(ackBackReply);
        (void) sentAckBytes;

      } else if (strcmp(tempBuff, "r") == 0) {
        char readingBuffer[2048];
        file->read(readingBuffer, 5);

        printf("Read: %s\n", readingBuffer);
      } else if (strcmp(tempBuff, "w") == 0) {
        printf("Enter data: ");
        fgets(tempBuff, sizeof tempBuff, stdin);
        tempBuff[strlen(tempBuff) - 1] = 0;
        size_t writtenBytes = file->write(tempBuff, strlen(tempBuff));
        printf("Written %zd\n", writtenBytes);
      } else if (strcmp(tempBuff, "l") == 0) {
        printf("Enter new position: ");
        fgets(tempBuff, sizeof tempBuff, stdin);
        tempBuff[strlen(tempBuff) - 1] = 0;
        printf("Temp Buff: %s\n", tempBuff);
        off_t offset = (off_t)atol(tempBuff);
        printf("%d\n", (int)offset);
        file->setOffset(offset);
        printf("Offset updated!\n");
      } else if (strcmp(tempBuff, "c") == 0) {
        printf("Closing..\n");
        fflush(stdout);
        printf("Client: File close %d\n", file->close());
      }
      continue;
    } else {
      requestMessage = Message(Request, tempBuff, _id, DEFAULT_MESSAGE_ID); //wrap the text in message form
    }

    uint32_t maxRetry = Settings::getInstance().getRetryTimes();

    while(!success && retry < (int)maxRetry){ //try to re-send packet if failed within MAX_RETRY
      printf("Sending %s..\n", tempBuff);
      fflush(stdout);


      sentBytes = _sendMessage(requestMessage); //send message to server

      if(sentBytes < 0)
      return -1;
      if(requestMessage.isTerminationMessage()){
        break;
      }

      uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
      Message ackReply = _getReplyTimeout(clientReplyTo, 0);

      if (ackReply.getType() != Acknowledge) {
        char invalidReplyMessage[LOG_MESSAGE_LENGTH];
        sprintf(invalidReplyMessage, "Invalid reply: %s", ackReply.getBytes());
        Logger::error(invalidReplyMessage);
        throw InvalidReplyException();
      }

      uint32_t ackNumber = (uint32_t) strtoull(ackReply.getBody(), NULL, 0);

      success = (ackNumber == sentBytes);


      sprintf(ackBack, "%d", (int) success);
      Message ackBackReply(Acknowledge, ackBack, _id, DEFAULT_MESSAGE_ID);

      sentAckBytes = _sendMessage(ackBackReply);
      (void) sentAckBytes;

      if(!success){ //packet loss
        char misacknowledgmentMessage[LOG_MESSAGE_LENGTH];
        sprintf(misacknowledgmentMessage, "Mismatch between acknowledgment and sent bytes (%d==%d)?.", (int)sentBytes, (int) ackNumber);
        Logger::error(misacknowledgmentMessage);
        retry++;
        if(retry < (int)maxRetry){
          char retryMessage[LOG_MESSAGE_LENGTH];
          sprintf(retryMessage, "Retrying to send the message(%d)..", retry);
          Logger::warn(retryMessage);
        }
      }
    }

    if(requestMessage.isTerminationMessage()){
      break;
    }

    if(!success){
      Logger::error("Failed to deliver your message.");
      continue;
    }



    printf("Receiving reply..\n");


    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
    Message replyMessage = _getReplyTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    const char *reply = replyMessage.getBody();
    printf("Reply: \"%s\"\n", reply);

  }
  return 0;*/
}

bool Client::reset() {
  stop();
  return true;
}

void Client::stop() {
  Thread::stop();
}

int Client::fetchResults(char *buf) {
  State currentState;
  Operation currentOperation;
  lock();
  currentOperation = _currentOperation;
  currentState = _resultState;
  unlock();
  if (currentState != Ready){
    if(currentOperation == Pinging)
      throw InvalidOperationContext();
    pause(&_fetchingCvLock);
  }
  _resultState = Steady;
  if(_resultState == Failed)
    return -1;
  strcpy(buf, _results);
  return 0;
}

void Client::setCommand(char *command) {
  strcpy(_queryParam, command);
}

void Client::queryRSA() {
  setCommand("rsa");
  start();
}

void Client::queryStegKey() {
  setCommand("steg");
  start();
}

const char *Client::getId() const {
  return _id;
}

State Client::checkState() const {
  return _resultState;
}

Message Client::_getReply() {
  return _clientSocket->getMessage();
}

Message Client::_getReplyTimeout(time_t seconds, suseconds_t mseconds) {
  return _clientSocket->recvMessageTimeout(seconds, mseconds);
}


ssize_t Client::_sendMessage(Message message){
  return _clientSocket->sendMessage(message);
}

Client::~Client() {
  pthread_mutex_destroy(&_operationLock);
  pthread_mutex_destroy(&_fetchingCvLock);
  pthread_cond_destroy(&_fetchingCv);
}
