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
  RSA *_pubKey;
  uint16_t _port;
  SeederJob *_reponderJob;

public:

  SeederNode(const char *id);

  SeederNode(const char *id, UDPSocket *socket, RSA *pubKey);

  SeederNode(const SeederNode &other);

  void setClientId(const char *id);

  const char *getClientId() const;

  UDPSocket *getSocket();

  void setSocket(UDPSocket *socket);

  void setPort(uint16_t port);

  uint16_t getPort();

  void setJob(SeederJob *job);

  SeederJob *getJob();

  RSA *getPublicKey();
  void setPublicKey(RSA *pubKey);

  const char *getUsername();
  void setUsername(const char *username);

  ~SeederNode();
};


#endif
