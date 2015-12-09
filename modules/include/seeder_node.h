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
  uint16_t _serverPort;
  SeederJob *_reponderJob;
  long _timestamp;
public:
  SeederNode(const char *publicKey);

  SeederNode(const char *publicKey, UDPSocket *socket);

  SeederNode(const SeederNode &other);

  void setClientId(const char *id);

  const char *getClientId();

  UDPSocket *getSocket();

  void setSocket(UDPSocket *socket);

  void setPort(uint16_t port);

  uint16_t getPort();

  void setServerPort(uint16_t port);

  uint16_t getServerPort() const;

  void setJob(SeederJob *job);

  SeederJob *getJob();

  const char *getPublicKey();
  void setPublicKey(const char *publicKey);

  const char *getUsername();
  void setUsername(const char *username);

  void getPeer(char* peer);

  long getTimestamp() const;

  bool isActive() const;

  ~SeederNode();
};


#endif
