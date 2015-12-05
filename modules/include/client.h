#ifndef CLIENT_H
#define CLIENT_H

#include "udp_socket.h"
#include "file.h"
#include "crypto.h"
#include "thread.h"


/*#define MAX_RETRY 3
#define CLIENT_REPLY_TO 3*/

class Client : public Thread {
private:
  UDPSocket * _clientSocket;
  uint16_t _port;

  char _hostname[128];
  char _username[128];
  char _publicRSA[2048];
  char _privateRSA[2048];
  char _id[128];

  Message _getReply();
  Message _getReplyTimeout(time_t seconds = 0, suseconds_t mseconds = 0);
  void _establishConnection(); //connect to server
  ssize_t _sendMessage(Message message);
public:
  Client(const char *username, const char * hostname, uint16_t port);
  void run();
  bool reset();
  void stop();
  int connect(); //

  ~Client();
};
#endif // CLIENT_H
