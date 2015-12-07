#include "heartbeat.h"

HeartBeat::HeartBeat(const char *username, const char * hostname, uint16_t port, uint16_t serverPort):Client(username, hostname, port, serverPort) {
  _currentOperation = Pinging;
  _resultState = Steady;

  pthread_condattr_init(&_timerAttr);
  pthread_condattr_setclock(&_timerAttr, CLOCK_MONOTONIC);

  if (pthread_mutex_init(&_timerMutex, NULL) != 0)
    throw MutexInitializationException();
  if (pthread_cond_init(&_timerCv, &_timerAttr))
    throw CVInitializationException();

  memset(_results, 0, MAX_READ_SIZE);

  _state = Disconnected;

}

//start sending messages from client.
void HeartBeat::run() {
  bool success; //success if message was sent wihtout any packet loss
  ssize_t sentBytes;  //number of bytes sent to the server
  int retry;
  Message pingMessage;

  uint32_t maxRetry = Settings::getInstance().getRetryTimes();
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

  _state = Connecting;
  if (_establishConnection() < 0) {
    printf("Connection failed!\n");
    return;
  }

  while(1) {
    Operation currentOperation;
    lock();
    currentOperation = _currentOperation;
    unlock();
    if (currentOperation == Pinging) {
      success = 0;
      retry = -1;

      pingMessage = Message(Ping, "1", _id, DEFAULT_MESSAGE_ID); //wrap the text in message form

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
          _state = Disconnected;
          if(retry < (int)maxRetry){
            char retryMessage[LOG_MESSAGE_LENGTH];
            sprintf(retryMessage, "Retrying to ping(%d)..", retry);
            Logger::warn(retryMessage);
            continue;
          } else {
            success = false;
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

      _waitTimer(PINGING_TIME);

    } else if (currentOperation == Querying) {
      _resultState = Fetching;
      Message queryMessage(Query, _queryParam, _id, DEFAULT_MESSAGE_ID);
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
  _resultState = Failed;
  resume();
}

void HeartBeat::queryOnline() {
  lock();
  _currentOperation = Querying;
  sprintf(_queryParam, "1");
  unlock();
  _wakeTimer();
}

void HeartBeat::queryUsername(char *username) {
  lock();
  _currentOperation = Querying;
  sprintf(_queryParam, "username=%s", username);
  unlock();
  _wakeTimer();
}

void HeartBeat::queryId(char *id) {
  lock();
  _currentOperation = Querying;
  sprintf(_queryParam, "id=%s", id);
  unlock();
  _wakeTimer();
}

bool HeartBeat::isConnected() const{
  return (_state == Connected);
}

bool HeartBeat::isConnecting() const {
  return (_state == Connecting);
}

bool HeartBeat::reset() {
  stop();
  return true;
}

void HeartBeat::stop() {
  Thread::stop();
}

void HeartBeat::_waitTimer(long waitVal) {
  clock_gettime(CLOCK_MONOTONIC, &_pingTime);
  _pingTime.tv_sec += waitVal;
  pthread_cond_timedwait(&_timerCv, &_timerMutex, &_pingTime);
}
void HeartBeat::_wakeTimer() {
  pthread_cond_signal(&_timerCv);
}


HeartBeat::~HeartBeat() {
  pthread_mutex_destroy(&_timerMutex);
  pthread_cond_destroy(&_timerCv);
}
