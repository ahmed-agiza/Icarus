#ifndef JOB_H
#define JOB_H

#include "thread.h"
#include "udp_socket.h"
#include "client_node.h"

class Job : public Thread {
  ClientNode *_client;
  char _id[128];
  char *_serverRSA;
  char *_serverStegKey;
  UDPSocket *_handlerSocket;
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

  void setStegKey(char *key);
  const char *getStegKey() const;

  void setId(char *id);

  void handleRemoteFile(Message &request);


  ~Job();
};

#endif
