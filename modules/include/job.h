#ifndef JOB_H
#define JOB_H

#include "thread.h"
#include "udp_socket.h"
#include "client_node.h"

class Job : public Thread {
  ClientNode *_client;
  void *_shared;
  char _id[128];
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

  //TO BE MERGED WITH THE THREAD CLASS!!!!!!!!!!!!
  void setSharedData(void *ptr);
  void *getSharedData() const;

  void setId(char *id);


  ~Job();
};

#endif
