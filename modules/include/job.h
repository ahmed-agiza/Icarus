#ifndef JOB_H
#define JOB_H

#include "thread.h"
#include "udp_socket.h"
#include "client_node.h"
#include "steganography.h"

class Job : public Thread {
  ClientNode *_client;
  char _id[128];
  char *_rsa;
  char *_serverRSA;
  char *_serverStegKey;
  UDPSocket *_handlerSocket;
  int _updateImageCount(char *fileDir, char *filePath, int newCount);
public:
  Job();
  Job(const char *id);
  Job(ClientNode *client);
  Job(const Job &other);

  void run();
  bool reset();
  void stop();

  ClientNode *getClient() const;
  void setClient(ClientNode *);

  void setServerRSA(char *key);
  const char *getServerRSA() const;

  void setRSA(char *key);
  const char *getRSA() const;

  void setStegKey(char *key);
  const char *getStegKey() const;

  void setId(char *id);

  void handleRemoteFile(Message &request);


  ~Job();
};

#endif
