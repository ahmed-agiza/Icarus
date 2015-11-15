#ifndef JOB_H
#define JOB_H

#include "thread.h"
#include "server_socket.h"

class Job : public Thread {
  ServerSocket *_socket;
  sockaddr_in _clientAddr;
  void *_shared;
public:
  Job(ServerSocket *handlerSocket);

  void run();

  ServerSocket *getSocket() const;

  void setSharedData(void *ptr);
  void *getSharedData() const;


  void setClientAddr (sockaddr_in clientAddr);
  sockaddr_in getClienAddr() const;

  ~Job();
};

#endif
