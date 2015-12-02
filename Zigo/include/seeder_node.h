#ifndef SEEDER_NODE_H
#define SEEDER_NODE_H

#include <map>
using std::map;

#include "udp_socket.h"
#include "crypto.h"

class SeederJob;

class SeederNode {
  char _clientId[128];
  char _username[128];
  UDPSocket *_socket;
  char _publicKey[2048];
  uint16_t _port;
  SeederJob *_reponderJob;

public:
  SeederNode(const char *publicKey);

  SeederNode(const char *publicKey, UDPSocket *socket);

  SeederNode(const SeederNode &other);

  void setClientId(const char *id);

  const char *getClientId() const;

  UDPSocket *getSocket();

  void setSocket(UDPSocket *socket);

  void setPort(uint16_t port);

  uint16_t getPort();

  void setJob(SeederJob *job);

  SeederJob *getJob();

  const char *getPublicKey();
  void setPublicKey(const char *publicKey);

  const char *getUsername();
  void setUsername(const char *username);

  ~SeederNode();
};


#endif
