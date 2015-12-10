#include "client.h"


Client::Client(const char *username, const char * hostname, uint16_t port, uint16_t serverPort): Thread(), _clientSocket(0), _connectionPort(port), _executed(false), _busy(false), _currentOperation(Idle), _serverPort(serverPort) {
  strcpy(_hostname, hostname);
  strcpy(_username, username);
  memset(_id, 0, 128);

  _resultsListener = NULL;
  _resultsParent = NULL;

  _progressListener = NULL;
  _progressParent = NULL;


  if (!File::exists(PUBLIC_KEY_PATH) || !File::exists(PRIVATE_KEY_PATH)) {
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
    fprintf(stderr, "%s\n", e.what());
    return -1;
  }
}

void Client::run() {
  _busy = true;
  if (_establishConnection() < 0) {
    if(_currentOperation == PingServer) {
      strcpy(_results, "0");
      _resultState = Ready;
      _currentOperation = Idle;
      resume();
      return;
    }
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
  _resultState = Fetching;
  ssize_t sentAckBytes;
  int success = 0;
  int retry = -1;
  char ackBack[2];
  bool terminated = false;
  (void) terminated;
  Operation activeOperation;

  lock();
  activeOperation = _currentOperation;
  unlock();
  uint32_t maxRetry = Settings::getInstance().getRetryTimes();
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

  if(activeOperation == RSARequest || activeOperation == StegKey || activeOperation == KeysRequest) {
    Message requestMessage = Message(Request, _queryParam, _id, DEFAULT_MESSAGE_ID);
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

    ThreadCallback listener = _resultsListener;
    void *parent = _resultsParent;
    _resultsListener = NULL;
    _resultsParent = NULL;

    printf("Callback()\n");
    fflush(stdout);
    if(listener) {
        printf("Not null\n");
        (*listener)(this, parent);
    }

    _resultState = Ready;
    _currentOperation = Idle;
    resume();
  } else if (activeOperation == SendFile) {
    char readBuff[1024];
    memset(readBuff, 0, 1024);
    FileState fileState;
    long currentSize;
    File *localFile = File::open(_queryParam, O_RDONLY);
    printf("FD: %d\n", localFile->getFd());
    if (localFile->getFd() < 0) {
        throw FileOpenException();
    }
    File *remoteFile = File::ropen(_clientSocket, _extraParam, _id, _extraParam, AttemptCreate, &fileState, &currentSize);
    printf("Current size: %ld\n", currentSize);
    fflush(stdout);
    if (currentSize > 0) {
      printf("Resuming..\n");
      localFile->setOffset((off_t) currentSize);
    }
    memset(_results, 0, MAX_READ_SIZE);
    sprintf(_results, "%d", (int)fileState);
    if(fileState == Locked) {
      printf("Locked!\n");
      throw FileLockedException();
    } else if (fileState != Opened) {
      printf("Remote! %d\n", (int)fileState);
      throw FileOpenException();
    }

    ProgressCallback listener = _progressListener;
    void *progressParent = _progressParent;

    off_t localOffset, remoteOffset;
    while(!localFile->isEOF()) {
      ssize_t readBytes = localFile->read(readBuff, 1024);
      remoteFile->write(readBuff, readBytes);
      localOffset = localFile->getOffset();
      remoteOffset = remoteFile->getOffset();
      float percentage = 100.0 * (float)remoteOffset/(float)localFile->getFileSize();
      if (percentage > 100.0)
        percentage = 100.0;
      printf("%f%%\n", percentage);
      if(listener)
          (*listener)(percentage, progressParent);

      if (!localFile->isEOF())
        if (localOffset != remoteOffset)
      localFile->setOffset(remoteFile->getOffset());
    }

    printf("File state: %d\ns", (int)Opened);

    localFile->close();
    remoteFile->close();

    _progressListener = NULL;
    _progressParent = NULL;

    _resultState = Ready;
    _currentOperation = Idle;

    resume();
  } else if (activeOperation == SendEncryptedFile) {
    printf("Sending encrypted file..\n");
    char readBuff[200], encryptedBuff[256];
    memset(readBuff, 0, 200);
    memset(encryptedBuff, 0, 256);
    FileState fileState;
    File *localFile = File::open(_queryParam, O_RDONLY);
    if (localFile->getFd() < 0)
      throw FileOpenException();
    File *remoteFile = File::ropen(_clientSocket, _extraParam, _id, _extraParam, AttemptCreate, &fileState);

    memset(_results, 0, MAX_READ_SIZE);
    sprintf(_results, "%d", (int)fileState);
    if(fileState == Locked) {
      printf("Locked!\n");
      throw FileLockedException();
    } else if (fileState != Opened) {
      printf("Remote!\n");
      throw FileOpenException();
    }

    off_t localOffset, remoteOffset;
    while(!localFile->isEOF()) {
      memset(readBuff, 0, 200);
      memset(encryptedBuff, 0, 256);
      ssize_t readBytes = localFile->read(readBuff, 200);

      (void)readBytes;
      int encryptionLength = Crypto::encrypt(_peerRSA, readBuff, encryptedBuff);
      remoteFile->write(encryptedBuff, readBytes, DEFAULT_MESSAGE_ID, true, encryptionLength);
      char temp[5000];
      Crypto::base64Encode(encryptedBuff, encryptionLength, temp, 5000);

      localOffset = localFile->getOffset();
      remoteOffset = remoteFile->getOffset();
      float percentage = 100.0 * (float)remoteOffset/(float)localFile->getFileSize();
      if (percentage > 100.0)
        percentage = 100.0;

      if (!localFile->isEOF())
        if (localOffset != remoteOffset)
          localFile->setOffset(remoteFile->getOffset());
    }

    printf("File state: %d\ns", (int)Opened);

    ThreadCallback listener = _resultsListener;
    void *parent = _resultsParent;
    _resultsListener = NULL;
    _resultsParent = NULL;

    if(listener) {
        (*listener)(this, parent);
    }

    localFile->close();
    remoteFile->close();

    _resultState = Ready;
    _currentOperation = Idle;

    resume();
  } else if (activeOperation == SendTempFile) {
    char readBuff[200], encryptedBuff[256];
    memset(readBuff, 0, 200);
    memset(encryptedBuff, 0, 256);
    FileState fileState;
    File *localFile = File::open(_queryParam, O_RDONLY);
    if (localFile->getFd() < 0)
        throw FileOpenException();
    File *remoteFile = File::ropen(_clientSocket, _extraParam, _id, _extraParam, AttemptCreate, &fileState);

    memset(_results, 0, MAX_READ_SIZE);
    sprintf(_results, "%d", (int)fileState);
    if(fileState == Locked) {
      printf("Locked!\n");
      throw FileLockedException();
    } else if (fileState != Opened) {
      printf("Remote!\n");
      throw FileOpenException();
    }

    off_t localOffset, remoteOffset;
    while(!localFile->isEOF()) {
      ssize_t readBytes = localFile->read(readBuff, 200);
      (void)readBytes;
      int encryptionLength = Crypto::encrypt(_peerRSA, readBuff, encryptedBuff);
      remoteFile->write(encryptedBuff, encryptionLength, DEFAULT_MESSAGE_ID, encryptionLength);
      localOffset = localFile->getOffset();
      remoteOffset = remoteFile->getOffset();
      float percentage = 100.0 * (float)remoteOffset/(float)localFile->getFileSize();
      if (percentage > 100.0)
      percentage = 100.0;

      if (!localFile->isEOF())
      if (localOffset != remoteOffset)
      localFile->setOffset(remoteFile->getOffset());
    }

    printf("File state: %d\ns", (int)Opened);

    localFile->close();
    remoteFile->close();

    ThreadCallback listener = _resultsListener;
    void *parent = _resultsParent;
    _resultsListener = NULL;
    _resultsParent = NULL;

    fflush(stdout);
    if(listener) {
        (*listener)(this, parent);
    }

    _resultState = Ready;
    _currentOperation = Idle;

    resume();
  } else if (activeOperation == UpdateImageViews) {
    char paramsBuffer[1024];
    sprintf(paramsBuffer, "%s;%s;", _queryParam, _extraParam);
    Message requestMessage = Message(UpdateImage, paramsBuffer, _id, DEFAULT_MESSAGE_ID);

    ssize_t sentBytes = _sendMessage(requestMessage); //send message to server

    if(sentBytes < 0)
      return -1;

    Message replyMessage = _getReplyTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    const char *reply = replyMessage.getBody();

    strcpy(_results, reply);
    printf("Reply: %s\n", reply);

    ThreadCallback listener = _resultsListener;
    void *parent = _resultsParent;
    _resultsListener = NULL;
    _resultsParent = NULL;

    printf("Callback()\n");
    fflush(stdout);
    if(listener) {
        printf("Not null\n");
        (*listener)(this, parent);
    }

    _resultState = Ready;
    _currentOperation = Idle;

    resume();
  } else if (activeOperation == PingServer) {
    Message requestMessage = Message(Ping, "1", _id, DEFAULT_MESSAGE_ID);

    ssize_t sentBytes = _sendMessage(requestMessage); //send message to server

    if(sentBytes < 0)
      return -1;
    Message replyMessage;
    try {
      replyMessage = _getReplyTimeout(clientReplyTo, 0);
    } catch (ReceiveTimeoutException &e) {
      strcpy(_results, "0");
      _resultState = Ready;
      _currentOperation = Idle;
      resume();
      return 0;
    }
    if (replyMessage.getType() != Pong) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      strcpy(_results, "0");
      throw InvalidReplyException();
    }

    strcpy(_results, "1");

    ThreadCallback listener = _resultsListener;
    void *parent = _resultsParent;
    _resultsListener = NULL;
    _resultsParent = NULL;

    printf("Callback()\n");
    fflush(stdout);
    if(listener) {
        printf("Not null\n");
        (*listener)(this, parent);
    }

    _resultState = Ready;
    _currentOperation = Idle;

    resume();
  } else {
    fprintf(stderr, "Invalid operation %d\n", (int) activeOperation);
  }



  return 0;

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

const char *Client::getUsername() {
    return _username;
}

const char *Client::getAddress() {
    return _hostname;
}

uint16_t Client::getConnectionPort() {
    return _connectionPort;
}

void Client::sendFile(const char *filename, const char *fileId) {
  lock();
  setCommand((char *) filename);
  setExtra((char *) fileId);
  printf("File name: %s\n", filename);
  printf("File ID: %s\n", fileId);
  _currentOperation = SendFile;
  unlock();
  start();
}

void Client::sendTempFile(const char *filename, const char *fileId) {
  lock();
  setCommand((char *) filename);
  setExtra((char *) fileId);
  _currentOperation = SendTempFile;
  unlock();
  start();
}

void Client::sendEncryptedFile(const char *filename, const char *fileId) {
  lock();
  setCommand((char* ) filename);
  setExtra((char *) fileId);
  _currentOperation = SendEncryptedFile;
  unlock();
  start();
}

void Client::updateImage(const char *fileId, const char *newCount) {
  lock();
  setCommand((char* ) fileId);
  setExtra((char *) newCount);
  _currentOperation = UpdateImageViews;
  unlock();
  start();
}

void Client::registerListener(ThreadCallback listener, void *parent) {
    _resultsListener = listener;
    _resultsParent = parent;
}

void Client::registerUpdateListener(ProgressCallback listener, void *parent) {
    _progressListener = listener;
    _progressParent = parent;
}

void Client::setPeerRSA(char *rsa) {
  memset(_peerRSA, 0, 2048);
  strcpy(_peerRSA, rsa);
}

void Client::setClientId(char *id) {
    memset(_clientId, 0, 128);
    strcpy(_clientId, id);
}

const char *Client::getClientId() {
    return _clientId;
}


void Client::setCommand(char *command) {
  strcpy(_queryParam, command);
}

void Client::setExtra(char *extra) {
  strcpy(_extraParam, extra);
}

void Client::queryRSA() {
  lock();
  _currentOperation = RSARequest;
  setCommand("rsa");
  unlock();
  start();
}

void Client::queryKeys() {
  lock();
  _currentOperation = RSARequest;
  setCommand("keys");
  unlock();
  start();
}

void Client::queryStegKey() {
  lock();
  _currentOperation = StegKey;
  setCommand("steg");
  unlock();
  start();
}

void Client::pingServer() {
  lock();
  _currentOperation = PingServer;
  unlock();
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
