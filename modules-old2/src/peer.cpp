#include "peer.h"

Peer::Peer() {
  memset(_id, 0, 128);
  memset(_address, 0, 128);
  memset(_rsa, 0, 2048);
}

Peer::Peer(char *id, char *address, char *rsa, char *username) {
  memset(_id, 0, 128);
  memset(_address, 0, 128);
  memset(_rsa, 0, 2048);
  strcpy(_id, id);
  strcpy(_address, address);
  if (rsa)
    strcpy(_rsa, rsa);
  if (username)
    strcpy(_username, username);
}

Peer::Peer(const Peer &other) {
  memset(_id, 0, 128);
    memset(_address, 0, 128);
  memset(_rsa, 0, 2048);
  strcpy(_id, other._id);
  strcpy(_address, other._address);
  if (other._rsa)
    strcpy(_rsa, other._rsa);
  if (_username)
    strcpy(_username, other._username);
}

Peer Peer::fromString(char *raw) {
  char id[128], user_name[128], ip[128], port[128];
  sscanf(raw, "%[^':']:%[^':']:%[^':']:%s", id, user_name, ip, port);
  Peer peer(id, ip, NULL, user_name);
  return peer;
}

PeersMap Peer::fromStringList(char *raw) {
/*<id>:<username>:<ip>:<port>*/
  PeersMap tempMap;
  char * pch;
  pch = strtok (raw,";");
  while (pch != NULL) {
    Peer tempPeer = fromString(pch);
    tempMap[(char *)tempPeer.getId()] = tempPeer;
    pch = strtok (NULL, ";");
  }
  return tempMap;
}

void Peer::setPeerAddress(const char *address) {
  strcpy(_address, address);
}
const char* Peer::getPeerAddress() const {
  return _address;
}

void Peer::setId(char *id) {
  strcpy(_id, id);
}
const char *Peer::getId() const {
  return _id;
}

void Peer::setUsername(char *username) {
  strcpy(_username, username);
}
const char *Peer::getUsername() const {
  return _username;
}

Peer::~Peer() {

}
