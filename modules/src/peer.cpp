#include "peer.h"

Peer::Peer() {
  memset(_id, 0, 128);
  memset(_address, 0, 128);
  memset(_rsa, 0, 2048);
  memset(_stegKey, 0, 2048);
}

Peer::Peer(char *id, char *address, char *rsa, char *username, uint16_t portNumber): _portNumber(portNumber) {
  memset(_id, 0, 128);
  memset(_address, 0, 128);
  memset(_rsa, 0, 2048);
  memset(_stegKey, 0, 2048);
  strcpy(_id, id);
  strcpy(_address, address);
  if (rsa)
    strcpy(_rsa, rsa);
  if (username)
    strcpy(_username, username);
}

Peer::Peer(const Peer &other) {
  _portNumber = other._portNumber;
  memset(_id, 0, 128);
  memset(_address, 0, 128);
  memset(_rsa, 0, 2048);
  memset(_stegKey, 0, 2048);
  strcpy(_id, other._id);
  strcpy(_address, other._address);
  strcpy(_stegKey, other._stegKey);
  if (other._rsa)
    strcpy(_rsa, other._rsa);
  if (_username)
    strcpy(_username, other._username);
}

Peer Peer::fromString(char *raw) {
  char id[128], username[128], ip[128], port[128];

  if(sscanf(raw, "%[^':']:%[^':']:%[^':']:%s", id, username, ip, port) != 4)
    throw InvalidMessageFormat();

  Peer peer(id, ip, NULL, username, (uint16_t) atoi(port));
  return peer;
}

PeersMap Peer::fromStringList(char *raw) {
  raw++;
  PeersMap tempMap;
  char * pch;
  pch = strtok (raw,";");
  while (pch != NULL) {
    if(strlen(pch) > 5) {
      try{
        Peer tempPeer = fromString(pch);
        char *id = new char[strlen(tempPeer.getId()) + 1];
        memset(id, 0, strlen(tempPeer.getId()) + 1);
        strcpy(id, tempPeer.getId());
        tempMap[id] = tempPeer;
      } catch (InvalidMessageFormat &e) {
        Logger::error(e.what());
      }
    }
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

void Peer::setRSA(char *rsa) {
  strcpy(_rsa, rsa);
}
const char *Peer::getRSA() const {
  return _rsa;
}

void Peer::setStegKey(char *key) {
  strcpy(_stegKey, key);
}
const char *Peer::getStegKey() const {
  return _stegKey;
}

void Peer::setPortNumber(uint16_t portNumber) {
  _portNumber = portNumber;
}
uint16_t Peer::getPortNumber() const {
  return _portNumber;
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
