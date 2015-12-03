#include "heartbeat.h"

HeartBeat::HeartBeat(const char * hostname, uint16_t port):Thread() {
  _currentOperation = Pinging;
  _resultState = Steady;

  memset(_id, 0, 128);
  memset(_results, 0, MAX_READ_SIZE);

  if (true || !File::exists(PUBLIC_KEY_PATH) || !File::exists(PRIVATE_KEY_PATH)) {
    Crypto::generateKeyPair(PRIVATE_KEY_PATH, PUBLIC_KEY_PATH);
  }

  File *publicKeyFile = File::open(PUBLIC_KEY_PATH, O_RDONLY);
  File *privateKeyFile = File::open(PRIVATE_KEY_PATH, O_RDONLY);
  memset(_publicRSA, 0, 2048);
  memset(_privateRSA, 0, 2048);
  publicKeyFile->read(_publicRSA, 2048);
  privateKeyFile->read(_privateRSA, 2048);
  publicKeyFile->close();
  privateKeyFile->close();
  delete publicKeyFile;
  delete privateKeyFile;

  _state = Disconnected;
  strcpy(_hostname, hostname);
  _clientSocket = new UDPSocket;
  _clientSocket->initialize(_hostname, port);
  _establishConnection();
  srand(time(NULL));

  if (pthread_mutex_init(&_operationLock, NULL) != 0)
    throw MutexInitializationException();
  if (pthread_mutex_init(&_fetchingCvLock, NULL) != 0)
    throw MutexInitializationException();
  if (pthread_cond_init(&_fetchingCv, NULL))
    throw CVInitializationException();
  setMutex(&_operationLock);
  setCV(&_fetchingCv);
}

void HeartBeat::_establishConnection() {
  pid_t pid = getpid();
  (void) pid;
  char connectionString[2048];
  sprintf(connectionString, "%s", _publicRSA);
  Crypto::md5Hash(_publicRSA, _id);
  printf("ID: %s\n", _id);

  printf("Connecting using %s\n", connectionString);
  Message connectionMessage = Message(Connect, connectionString);
  ssize_t sentBytes = _sendMessage(connectionMessage);

  if(sentBytes < 0)
    return;
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
  printf("Attempting to connect..\n");
  Message portReply = _getReplyTimeout(clientReplyTo, 0);
  printf("Received!\n");
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
}

//start sending messages from client.
void HeartBeat::run() {
  printf("run()\n");
  bool success; //success if message was sent wihtout any packet loss
  ssize_t sentBytes;  //number of bytes sent to the server
  int retry;
  Message pingMessage;

  uint32_t maxRetry = Settings::getInstance().getRetryTimes();
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

  while(1) {
    Operation currentOperation;
    lock();
    currentOperation = _currentOperation;
    unlock();
    if (currentOperation == Pinging) {
      success = 0;
      retry = -1;

      pingMessage = Message(Ping, "1"); //wrap the text in message form

      while(!success && retry < (int)maxRetry) { //try to re-send packet if failed within MAX_RETRY

        sentBytes = _sendMessage(pingMessage); //send message to server

        if(sentBytes < 0) {
          success = false;
          continue;
        }

        Message pongReply;

        try {
          pongReply = _getReplyTimeout(clientReplyTo, 0);
        } catch (ReceiveTimeoutException &e) {
          retry++;
          if(retry < (int)maxRetry){
            char retryMessage[LOG_MESSAGE_LENGTH];
            sprintf(retryMessage, "Retrying to ping(%d)..", retry);
            Logger::warn(retryMessage);
            continue;
          } else {
            success = false;
            _state = Disconnected;
            break;
          }
        }
        if (pongReply.getType() != Pong) {
          char invalidReplyMessage[LOG_MESSAGE_LENGTH];
          sprintf(invalidReplyMessage, "Invalid reply: %s", pongReply.getBytes());
          Logger::error(invalidReplyMessage);
          throw InvalidReplyException();
        }
        //printf("Received pong!\n");
        _state = Connected;
        success = true;
        break;

      }
      if(!success){
        Logger::error("Failed to ping.");
        _state = Disconnected;
        break;
      }
      sleep(5);

    } else if (currentOperation == Querying) {
      _resultState = Fetching;
      Message queryMessage(Query, "1");
      sentBytes = _sendMessage(queryMessage); //send message to server
      Message resultReply;
      try {
        resultReply = _getReplyTimeout(clientReplyTo, 0);
      } catch (ReceiveTimeoutException &e) {
        success = false;
        _state = Disconnected;
        break;
      }
      if (resultReply.getType() != Reply) {
        char invalidReplyMessage[LOG_MESSAGE_LENGTH];
        sprintf(invalidReplyMessage, "Invalid reply: %s", resultReply.getBytes());
        Logger::error(invalidReplyMessage);
        throw InvalidReplyException();
      }
      strcpy(_results, resultReply.getBody());
      _resultState = Ready;
      resume();
      lock();
      _currentOperation = Pinging;
      unlock();
      _state = Connected;
    }
  }
}

void HeartBeat::queryOnline() {
  lock();
  _currentOperation = Querying;
  unlock();
}
void HeartBeat::fetchResults(char *buf) {
  State currentState;
  Operation currentOperation;
  lock();
  currentOperation = _currentOperation;
  currentState = _resultState;
  unlock();
  if (currentState != Ready){
    if(currentOperation != Querying)
      throw InvalidOperationContext();
    pause(&_fetchingCvLock);
  }
  _resultState = Steady;
  strcpy(buf, _results);
}

bool HeartBeat::reset() {
  stop();
  return true;
}

void HeartBeat::stop() {
  Thread::stop();
}

Message HeartBeat::_getReply() {
  return _clientSocket->getMessage();
}

Message HeartBeat::_getReplyTimeout(time_t seconds, suseconds_t mseconds) {
  return _clientSocket->recvMessageTimeout(seconds, mseconds);
}


ssize_t HeartBeat::_sendMessage(Message message){
  return _clientSocket->sendMessage(message);
}

HeartBeat::~HeartBeat() {
  pthread_mutex_destroy(&_operationLock);
  pthread_mutex_destroy(&_fetchingCvLock);
}
