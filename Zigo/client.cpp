#include "client.h"


Client::Client(const char * hostname, uint16_t port) {
  strcpy(_hostname, hostname);
  _clientSocket = new UDPSocket();
  _clientSocket->initialize(_hostname, port);
  _establishConnection();
  srand(time(NULL));
}

void Client::_establishConnection() {
  pid_t pid = getpid();
  char connectionString[64];
  sprintf(connectionString, "%s:%ld", "127.0.0.1", (long) pid);

  printf("Connecting using %s\n", connectionString);
  Message connectionMessage = Message(Connect, connectionString);
  ssize_t sentBytes = _sendMessage(connectionMessage);

  if(sentBytes < 0)
  return;

  Message portReply = _getReplyTimeout(CLIENT_REPLY_TO, 0);
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
int Client::start() {
  char tempBuff[2048]; //max char from user input
  char ackBack[2];
  bool success; //success if message was sent wihtout any packet loss
  ssize_t sentBytes;  //number of bytes sent to the server
  ssize_t sentAckBytes;
  int retry;
  Message requestMessage;
  while(1){
    success = 0;
    retry = -1;
    printf("Enter message to send:\n");
    fgets(tempBuff, sizeof tempBuff, stdin);
    tempBuff[strlen(tempBuff) - 1] = 0;

    while(!success && retry < MAX_RETRY){ //try to re-send packet if failed within MAX_RETRY
      printf("Sending %s..\n", tempBuff);
      fflush(stdout);

      requestMessage = Message(Request, tempBuff); //wrap the text in message form

      sentBytes = _sendMessage(requestMessage); //send message to server

      if(sentBytes < 0)
      return -1;
      if(requestMessage.isTerminationMessage()){
        break;
      }

      Message ackReply = _getReplyTimeout(CLIENT_REPLY_TO, 0);

      if (ackReply.getType() != Acknowledge) {
        char invalidReplyMessage[LOG_MESSAGE_LENGTH];
        sprintf(invalidReplyMessage, "Invalid reply: %s", ackReply.getBytes());
        Logger::error(invalidReplyMessage);
        throw InvalidReplyException();
      }

      uint32_t ackNumber = (uint32_t) strtoull(ackReply.getBody(), NULL, 0);

      success = (ackNumber == sentBytes);


      sprintf(ackBack, "%d", (int) success);
      Message ackBackReply(Acknowledge, ackBack);

      sentAckBytes = _sendMessage(ackBackReply);

      if(!success){ //packet loss
        char misacknowledgmentMessage[LOG_MESSAGE_LENGTH];
        sprintf(misacknowledgmentMessage, "Mismatch between acknowledgment and sent bytes (%d==%d)?.", (int)sentBytes, (int) ackNumber);
        Logger::error(misacknowledgmentMessage);
        retry++;
        if(retry < MAX_RETRY){
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
    Message replyMessage = _getReplyTimeout(CLIENT_REPLY_TO, 0);
    if (replyMessage.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    const char *reply = replyMessage.getBody();
    printf("Reply: \"%s\"\n", reply);

  }
  return 0;
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

}
