#ifndef JOB_H
#define JOB_H

#include "thread.h"
#include "udp_socket.h"
#include "client_node.h"

class Job : public Thread {
  ClientNode *_client;
  void *_shared;
public:
  Job();
  Job(ClientNode *client);
  Job(const Job &other);

  void run();
  bool reset();
  void stop();

  ClientNode *getClient() const;
  void setClient(ClientNode *);

  void setSharedData(void *ptr);
  void *getSharedData() const;


  ~Job();
};

#endif
