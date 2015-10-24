#include "client.h"

#define TEST_FAIL
#undef TEST_FAIL

Client::Client(char * hostname, uint16_t port) {
  strcpy(_hostname, hostname);
  _clientSocket = new ClientSocket();
  _clientSocket->initializeClient(_hostname, port);
  _establishConnection();

#ifdef TEST_FAIL
  //Failure testing----------------------------------------------------------
  _clientSocket = new ClientSocket();
  _clientSocket->initializeClient(_hostname, port);
  _establishConnection();
#endif

  srand(time(NULL));
}

void Client::_establishConnection() {
  pid_t pid = getpid();
  char connectionString[64];
  sprintf(connectionString, "%s:%ld", "127.0.0.1", (long) pid);

  _clientSocket->setTimeout(3, 0);

  printf("Connecting using %s\n", connectionString);
  ssize_t sentBytes = _sendRawMessage(connectionString);
  if(sentBytes < 0)
    return;

  const char *port = _getRawReply();

  _port = (uint16_t)strtoull(port, NULL, 0);
  printf("Connection port: %u\n", _port);
  delete _clientSocket;
  _clientSocket = new ClientSocket();
  _clientSocket->initializeClient(_hostname, _port);
  _clientSocket->setRecvTimeout(5, 0);
}

int Client::start() {
  char tempBuff[2048];
  char terminationString[] = "q";
  char ackBack[2];
  bool success;
  ssize_t sentBytes;
  int retry;
  while(1){
    success = 0;
    retry = -1;
    printf("Enter message to send:\n");
    fgets(tempBuff, sizeof tempBuff, stdin);
    tempBuff[strlen(tempBuff) - 1] = 0;

    while(!success && retry < MAX_RETRY){
        printf("Sending %s..\n", tempBuff);
        fflush(stdout);

        Message requestMessage(Request, strlen(tempBuff), tempBuff);

        sentBytes = _sendMessage(requestMessage);
        if(sentBytes < 0)
          return -1;
        if(strcmp(tempBuff, terminationString) == 0){
          break;
        }

        const char *ack = _getRawReply();
        uint32_t ackNumber = (uint32_t) strtoull(ack, NULL, 0);

#ifdef TEST_FAIL
        //Failure testing----------------------------------------------------------
        int dropped = rand() % 10;
        printf("Error(p) = %d\n", dropped);
        if(dropped == 5 || dropped == 3)
            ackNumber--;
#endif
        delete ack;

        success = (ackNumber == sentBytes);
        sprintf(ackBack, "%d", (int) success);
        _sendRawMessage(ackBack);

        if(!success){
          fprintf(stderr, "Mismatch between acknowledgment and sent bytes (%d==%d)?.\n", (int)sentBytes, (int) ackNumber);
          retry++;
          if(retry < MAX_RETRY)
            printf("Retrying to send the message(%d)..\n", retry);
        }
    }

    if(strcmp(tempBuff, terminationString) == 0){
      break;
    }

    if(!success){
        fprintf(stderr, "Failed to deliver your message.\n");
        continue;
    }



    printf("Receiving reply..\n");
    Message replyMessage = _getReply();
    const char *reply = replyMessage.getBody();
    printf("Reply: \"%s\"\n", reply);

  }
  return 0;
}

const char *Client::_getRawReply() {
  return _clientSocket->getRawMessage();
}

Message Client::_getReply() {
  return _clientSocket->getMessage();
}

ssize_t Client::_sendRawMessage(char * message){
  return _clientSocket->sendRaw(message, strlen(message));
}

ssize_t Client::_sendMessage(Message message){
  const char *bytes = message.getBytes();
  ssize_t sentBytes = _clientSocket->sendRaw(bytes, strlen(bytes));
  delete bytes;
  return sentBytes;
}

Client::~Client() {

}
