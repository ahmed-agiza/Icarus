#ifndef PEER_H
#define PEER_H

#include <map>
using std::map;

#include "udp_socket.h"

#ifndef STRING_COMPARE_OPERATOR
#define STRING_COMPARE_OPERATOR
struct StringCompare {
   bool operator()(char const *a, char const *b) {
      return (strcmp(a, b) < 0);
   }
};
#endif

class Peer;

typedef map<char *, Peer, StringCompare> PeersMap;

class Peer {
  char _address[128];
  char _id[128];
  char _username[128];
  char _rsa[2048];
  char _stegKey[2048];
  uint16_t _portNumber;
public:
  Peer();
  Peer(char *id, char *address, char *rsa = 0, char *username = 0, uint16_t portNumber = 0);
  Peer(const Peer &other);
  static Peer fromString(char *raw);
  static PeersMap fromStringList(char *raw);

  void setPeerAddress(const char *address);
  const char* getPeerAddress() const;

  void setPortNumber(uint16_t portNumber);
  uint16_t getPortNumber() const;

  void setId(char *id);
  const char *getId() const;

  void setRSA(char *rsa);
  const char *getRSA() const;

  void setStegKey(char *key);
  const char *getStegKey() const;

  void setUsername(char *username);
  const char *getUsername() const;

  ~Peer();
};

#endif
