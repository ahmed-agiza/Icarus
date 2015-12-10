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
  KeysRequest = 5,
  OpenFile = 6,
  SendFile = 7,
  SendTempFile = 8,
  SendEncryptedFile = 9,
  UpdateImageViews = 10,
  PingServer = 11
};

enum State {
  Steady = 0,
  Fetching = 1,
  Ready = 2,
  Failed = 3
};


typedef void (*ProgressCallback)(float, void*);


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
  char _clientId[128];
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

  ThreadCallback _resultsListener;
  void *_resultsParent;

  ProgressCallback _progressListener;
  void *_progressParent;

public:
  Client(const char *username, const char * hostname, uint16_t port, uint16_t serverPort);
  bool reset();
  void stop();
  int execute();

  void run();

  virtual int fetchResults(char *buf);

  const char *getUsername();
  const char *getAddress();
  uint16_t getConnectionPort();


  void setCommand(char *command);
  void queryRSA();
  void queryStegKey();
  void queryKeys();

  const char *getId() const;

  State checkState() const;

  void sendFile(const char *filename, const char *fileId);
  void sendEncryptedFile(const char *filename, const char *fileId);
  void sendTempFile(const char *filename, const char *fileId);
  void pingServer();
  void updateImage(const char *fileId, const char *newCount);

  void registerListener(ThreadCallback listener, void *parent);

  void registerUpdateListener(ProgressCallback listener, void *parent);

  void setExtra(char *extra);
  void setPeerRSA(char *rsa);

  void setClientId(char *id);
  const char *getClientId();


  ~Client();
};
#endif // CLIENT_H
