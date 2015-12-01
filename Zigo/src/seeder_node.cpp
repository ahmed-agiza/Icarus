#include "seeder_node.h"


SeederNode::SeederNode(const char *id): _socket(0) {
  strcpy(_clientId, id);
}

SeederNode::SeederNode(const char *id, UDPSocket *socket, RSA *pubKey): _socket(socket), _pubKey(pubKey) {
  strcpy(_clientId, id);
}

SeederNode::SeederNode(const SeederNode &other): _socket(other._socket){
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

RSA *SeederNode::getPublicKey() {
  return _pubKey;
}
void SeederNode::setPublicKey(RSA *pubKey) {
  _pubKey = pubKey;
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
