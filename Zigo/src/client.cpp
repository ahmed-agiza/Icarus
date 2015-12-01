#include "client.h"


Client::Client(const char * hostname, uint16_t port) {
  strcpy(_hostname, hostname);
  _clientSocket = new UDPSocket;
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
  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
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
}

//start sending messages from client.
int Client::start() {
  char tempBuff[2048]; //max char from user input
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
        file = File::ropen(_clientSocket, tempBuff);
        int fd = file->getFd();
        printf("File descriptor: %d\n", fd);

        success = 1;

        sprintf(ackBack, "%d", (int) success);
        Message ackBackReply(Acknowledge, ackBack);

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
      requestMessage = Message(Request, tempBuff); //wrap the text in message form
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
      Message ackBackReply(Acknowledge, ackBack);

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
