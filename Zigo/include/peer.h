#ifndef PEER_H
#define PEER_H

#include <map>
using std::map;

#include "udp_socket.h"

struct StringCompare {
   bool operator()(char const *a, char const *b) {
      return (strcmp(a, b) != 0);
   }
};

class Peer;

typedef map<char *, Peer, StringCompare> PeersMap;

class Peer {
  char _address[128];
  char _id[128];
  char _username[128];
  char _rsa[2048];
public:
  Peer();
  Peer(char *id, char *address, char *rsa = 0, char *username = 0);
  Peer(const Peer &other);
  static Peer fromString(char *raw);
  static PeersMap fromStringList(char *raw);

  void setPeerAddress(const char *address);
  const char* getPeerAddress() const;

  void setId(char *id);
  const char *getId() const;

  void setUsername(char *username);
  const char *getUsername() const;

  ~Peer();
};

#endif
