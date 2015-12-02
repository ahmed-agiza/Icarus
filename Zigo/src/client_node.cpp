#include "client_node.h"


ClientNode::ClientNode(const char *id): _socket(0) {
  memset(_clientId, 0, 128);
  strcpy(_clientId, id);
}

ClientNode::ClientNode(const char *id, UDPSocket *socket): _socket(socket) {
  memset(_clientId, 0, 128);
  strcpy(_clientId, id);
}

ClientNode::ClientNode(const ClientNode &other): _socket(other._socket){
  memset(_clientId, 0, 128);
  strcpy(_clientId, other._clientId);
}

void ClientNode::setClientId(const char *id) {
  memset(_clientId, 0, 128);
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

File *ClientNode::addFile(int fd, File *file) {
  _openFiles[fd] = file;
  return file;
}

File *ClientNode::getFile(int fd) {
  if(hasOpenFile(fd)) {
    return _openFiles[fd];
  }

  return NULL;
}

bool ClientNode::removeFile(int fd) {
  if(hasOpenFile(fd)) {
    _openFiles.erase(fd);
    return true;
  }

  return false;
}

bool ClientNode::hasOpenFile(int fd) {
  if(_openFiles.find(fd) != _openFiles.end()) {
    return _openFiles[fd]->isOpen();
  }
  return false;
}

map<int, File *> &ClientNode::getFileTable() {
  return _openFiles;
}
ClientNode::~ClientNode() {
  for (std::map<int, File *>::iterator it = _openFiles.begin(); it != _openFiles.end(); ++it) {
    if (it->second && it->second->isOpen()) {
        it->second->close();
        delete it->second;
    }
  }
}
