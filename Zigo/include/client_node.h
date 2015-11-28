#ifndef CLIENT_NODE_H
#define CLIENT_NODE_H

#include "udp_socket.h"

class Job;

//A client tree structure.
class ClientNode {
  char _clientId[128];
  UDPSocket *_socket;
  uint16_t _port;
  Job *_reponderJob;

public:

  ClientNode(const char *id);

  ClientNode(const char *id, UDPSocket *socket);

  ClientNode(const ClientNode &other);

  void setClientId(const char *id);

  const char *getClientId() const;

  UDPSocket *getSocket();

  void setSocket(UDPSocket *socket);

  void setPort(uint16_t port);

  uint16_t getPort();


  void setJob(Job *job);

  Job *getJob();


};

#endif
