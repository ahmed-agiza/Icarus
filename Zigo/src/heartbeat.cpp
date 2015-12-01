#include "heartbeat.h"


HeartBeat::HeartBeat(const char * hostname, uint16_t port):Thread() {
  _state = Disconnected;
  strcpy(_hostname, hostname);
  _clientSocket = new UDPSocket;
  _clientSocket->initialize(_hostname, port);
  _establishConnection();
  srand(time(NULL));
}

void HeartBeat::_establishConnection() {
  pid_t pid = getpid();
  char connectionString[64];
  sprintf(connectionString, "%s:%ld", "127.0.0.1", (long) pid);

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
  printf("run()");
  bool success; //success if message was sent wihtout any packet loss
  ssize_t sentBytes;  //number of bytes sent to the server
  int retry;
  Message pingMessage;

  uint32_t maxRetry = Settings::getInstance().getRetryTimes();
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

  while(1) {
    success = 0;
    retry = -1;

    pingMessage = Message(Ping, "1"); //wrap the text in message form


    while(!success && retry < (int)maxRetry) { //try to re-send packet if failed within MAX_RETRY
      printf("Pinging..\n");
      fflush(stdout);


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
      printf("Received pong!\n");

      _state = Connected;

    }


    if(!success){
      Logger::error("Failed to ping.");
      _state = Disconnected;
      break;
    }

    sleep(5);

  }

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

}
