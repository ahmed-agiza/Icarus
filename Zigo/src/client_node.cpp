#include "client_node.h"


ClientNode::ClientNode(const char *id): _socket(0) {
  strcpy(_clientId, id);
}

ClientNode::ClientNode(const char *id, UDPSocket *socket): _socket(socket) {
  strcpy(_clientId, id);
}

ClientNode::ClientNode(const ClientNode &other): _socket(other._socket){
  strcpy(_clientId, other._clientId);
}

void ClientNode::setClientId(const char *id) {
  strcpy(_clientId, id);
}

const char *ClientNode::getClientId() const {
  return _clientId;
}

UDPSocket *ClientNode::getSocket() {
  return _socket;
}
void ClientNode::setSocket(UDPSocket *socket) {
  _socket = socket;
}

void ClientNode::setPort(uint16_t port) {
  _port = port;
}

uint16_t ClientNode::getPort() {
  return _port;
}


void ClientNode::setJob(Job *job) {
  _reponderJob = job;
}

Job *ClientNode::getJob(){
  return _reponderJob;
}
