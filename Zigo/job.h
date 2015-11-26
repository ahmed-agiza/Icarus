#ifndef JOB_H
#define JOB_H

#include "thread.h"
#include "udp_socket.h"

class Job : public Thread {
  UDPSocket *_socket;
  sockaddr_in _clientAddr;
  void *_shared;
public:
  Job(UDPSocket *handlerSocket = 0);
  Job(const Job &other);

  void run();
  bool reset();
  void stop();

  UDPSocket *getSocket() const;
  void setSocket(UDPSocket *socket);

  void setSharedData(void *ptr);
  void *getSharedData() const;


  void setClientAddr (sockaddr_in clientAddr);
  sockaddr_in getClienAddr() const;

  ~Job();
};

#endif
