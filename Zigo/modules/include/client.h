#ifndef CLIENT_H
#define CLIENT_H

#include "udp_socket.h"
#include "file.h"
#include "crypto.h"
#include "thread.h"


enum Operation {
  Idle = 0,
  Pinging = 1,
  Querying = 2,
  RSARequest = 3,
  StegKey = 4,
  OpenFile = 5,
  SendFile = 6,
  SendEncryptedFile = 7
};

enum State {
  Steady = 0,
  Fetching = 1,
  Ready = 2,
  Failed = 3
};


class Client : public Thread {
protected:
  UDPSocket * _clientSocket;
  uint16_t _connectionPort;
  uint16_t _port;

  char _hostname[128];
  char _username[128];
  char _publicRSA[2048];
  char _privateRSA[2048];
  char _id[128];
  char _peerRSA[2048];

  bool _executed;
  bool _busy;


  char _queryParam[2048];
  char _extraParam[2048];
  char _results[MAX_READ_SIZE];
  Operation _currentOperation;
  State _resultState;
  uint16_t _serverPort;
  pthread_mutex_t _operationLock;
  pthread_mutex_t _fetchingCvLock;
  pthread_cond_t _fetchingCv;

  Message _getReply();
  Message _getReplyTimeout(time_t seconds = 0, suseconds_t mseconds = 0);
  int _establishConnection(); //connect to server
  ssize_t _sendMessage(Message message);
public:
  Client(const char *username, const char * hostname, uint16_t port, uint16_t serverPort);
  bool reset();
  void stop();
  int execute();

  void run();

  virtual int fetchResults(char *buf);

  void setCommand(char *command);
  void queryRSA();
  void queryStegKey();

  const char *getId() const;

  State checkState() const;

  void sendFile(const char *filename, const char *fileId);
  void sendEncryptedFile(const char *filename, const char *fileId);

  void setExtra(char *extra);
  void setPeerRSA(char *rsa);


  ~Client();
};
#endif // CLIENT_H
