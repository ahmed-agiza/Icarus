#include "seeder_node.h"

SeederNode::SeederNode(const char *publicKey): _socket(0) {
  memset(_clientId, 0, 128);
  memset(_username, 0, 128);
  memset(_publicKey, 0, 128);

  strcpy(_publicKey, publicKey);
  Crypto::md5Hash(_publicKey, _clientId);
}

SeederNode::SeederNode(const char *publicKey, UDPSocket *socket): _socket(socket) {
  memset(_clientId, 0, 128);
  memset(_username, 0, 128);
  memset(_publicKey, 0, 128);

  strcpy(_publicKey, publicKey);
  Crypto::md5Hash(_publicKey, _clientId);
}

SeederNode::SeederNode(const SeederNode &other): _socket(other._socket){
  strcpy(_publicKey, other._publicKey);
  strcpy(_clientId, other._clientId);
}

void SeederNode::setClientId(const char *id) {
  strcpy(_clientId, id);
}

const char *SeederNode::getClientId() const {
  return _clientId;
}

UDPSocket *SeederNode::getSocket() {
  return _socket;
}
void SeederNode::setSocket(UDPSocket *socket) {
  _socket = socket;
}

void SeederNode::setPort(uint16_t port) {
  _port = port;
}

uint16_t SeederNode::getPort() {
  return _port;
}

const char *SeederNode::getPublicKey() {
  return _publicKey;
}

void SeederNode::setPublicKey(const char *publicKey) {
  strcpy(_publicKey, publicKey);
}

void SeederNode::setJob(SeederJob *job) {
  _reponderJob = job;
}

SeederJob *SeederNode::getJob(){
  return _reponderJob;
}

const char *SeederNode::getUsername() {
  return _username;
}

void SeederNode::setUsername(const char *username) {
  strcpy(_username, username);
}

SeederNode::~SeederNode() {

}
