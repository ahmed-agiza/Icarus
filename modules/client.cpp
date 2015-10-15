#include "client.h"

Client::Client(char * hostname, uint16_t port) {
  strcpy(_hostname, hostname);
  _clientSocket = new ClientSocket();
  _clientSocket->initializeClient(_hostname, port);
  _establishConnection();
}

void Client::_establishConnection() {
  pid_t pid = getpid();
  char connectionString[64];
  sprintf(connectionString, "%s:%ld", "127.0.0.1", (long) pid);
  printf("Connecting using %s\n", connectionString);
  ssize_t sentBytes = _sendMessage(connectionString);
  if(sentBytes < 0)
    return;

  const char *port = _getReply();

  _port = (uint16_t)strtoull(port, NULL, 0);
  printf("Connection port: %u\n", _port);
  delete _clientSocket;
  _clientSocket = new ClientSocket();
  _clientSocket->initializeClient(_hostname, _port);
}

int Client::start() {
  char tempBuff[2048];
  char terminationString[] = "q";
  bool success = 1;
  ssize_t sentBytes;
  while(1){
    printf("Enter message to send:\n");
    fgets(tempBuff, sizeof tempBuff, stdin);
    tempBuff[strlen(tempBuff) - 1] = 0;
    printf("Sending %s..\n", tempBuff);
    fflush(stdout);
    sentBytes = _sendMessage(tempBuff);
    if(sentBytes < 0)
      return -1;
    if(strcmp(tempBuff, terminationString) == 0){
      break;
    }

    const char *ack = _getReply();
    delete ack;
    uint32_t ackNumber = (uint32_t) strtoull(ack, NULL, 0);

    success = (ackNumber == sentBytes);

    if(!success){
      fprintf(stderr, "Mismatch between acknowledgment and sent bytes.\n");
    }

    printf("Receiving reply..\n");
    const char *reply = _getReply();
    printf("Reply: received %s.\n", reply);
    delete reply;

  }
  return 0;
}

const char *Client::_getReply() {
  return _clientSocket->getMessage();
}

ssize_t Client::_sendMessage(char * message){
  return _clientSocket->sendRaw(message, strlen(message));
}

Client::~Client() {

}
