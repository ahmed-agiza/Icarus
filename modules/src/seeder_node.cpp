#include "seeder_node.h"

SeederNode::SeederNode(const char *publicKey): _socket(0), _timestamp((long)time(NULL)){
  _serverPort = 0;
  memset(_clientId, 0, 128);
  memset(_username, 0, 128);
  memset(_publicKey, 0, 128);

  strcpy(_publicKey, publicKey);
  Crypto::md5Hash(_publicKey, _clientId);
}

SeederNode::SeederNode(const char *publicKey, UDPSocket *socket): _socket(socket), _timestamp((long)time(NULL)) {
  printf("SeederNode::SeederNode(const char *publicKey, UDPSocket *socket)\n");
  memset(_clientId, 0, 128);
  memset(_username, 0, 128);
  memset(_publicKey, 0, 128);

  strcpy(_publicKey, publicKey);
  Crypto::md5Hash(_publicKey, _clientId);
}

SeederNode::SeederNode(const SeederNode &other): _socket(other._socket), _timestamp(other._timestamp), _serverPort(other._serverPort){
  printf("WTH: SeederNode::SeederNode(const SeederNode &other): %s\n", other._username);
  strcpy(_publicKey, other._publicKey);
  strcpy(_clientId, other._clientId);
  strcpy(_username, other._username);
}

void SeederNode::setClientId(const char *id) {
  strcpy(_clientId, id);
}

const char *SeederNode::getClientId() {
  return _clientId;
}

UDPSocket *SeederNode::getSocket() {
  return _socket;
}
void SeederNode::setSocket(UDPSocket *socket) {
  printf("setSokcet(%d)\n", (int)(socket != 0));
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

void SeederNode::getPeer(char* peer) {
  memset(peer, 0, 2048);
  char port[128];
  strcat(peer, getClientId());
  strcat(peer, ":");
  printf("Username(getPeer()): %s\n", getUsername());
  strcat(peer, getUsername());
  strcat(peer, ":");
  if (_socket)
    strcat(peer, _socket->getPeerName());
  else {
    Logger::warn("Socket is not constructed!");
    strcat(peer, "-");
  }
  strcat(peer, ":");
  sprintf(port, "%d", (int)getServerPort());
  strcat(peer, port);
}

bool SeederNode::isActive() const {
  return (_socket != NULL && _socket != 0);
}

long SeederNode::getTimestamp() const {
  return _timestamp;
}

void SeederNode::setServerPort(uint16_t port) {
  _serverPort = port;
}

uint16_t SeederNode::getServerPort() const {
  return _serverPort;
}

SeederNode::~SeederNode() {

}
