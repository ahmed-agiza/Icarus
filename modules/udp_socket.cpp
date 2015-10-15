#include "udp_socket.h"

UDPSocket::UDPSocket () {
  _socketFd = socket(AF_INET, SOCK_DGRAM, 0);
  if(_socketFd < 0)
    throw "Failed to create UDP socket.\n";
  _sinSize = sizeof(sockaddr_in);
}

int UDPSocket::getFd() const {
  return _socketFd;
}

ssize_t UDPSocket::sendRaw(const char *data, ssize_t length) const {
  return sendRaw(data, length, _peerAddr);
}

ssize_t UDPSocket::sendRaw(const char *data, ssize_t length, const sockaddr_in &destinationAddr) const {
  return sendto(_socketFd, data, length + 1, 0, (sockaddr *) &destinationAddr, _sinSize);
}

ssize_t UDPSocket::recvRaw(ssize_t length) {
  return recvRaw(length, _peerAddr);
}

ssize_t UDPSocket::recvRaw(ssize_t length, sockaddr_in &sourceAddr) {
  return recvfrom(_socketFd, _readBuff, length, 0, (sockaddr *) &sourceAddr, &_sinSize);
}

const char *UDPSocket::getReadBuff () const {
  return _readBuff;
}

const char *UDPSocket::getRawMessage () {
  ssize_t recvBytes = recvRaw();
  if(recvBytes < 0)
    fprintf(stderr, "%s\n", "An error occured while receiving the message.");
  char *recvData = new char[recvBytes];
  strcpy(recvData, getReadBuff());
  return recvData;
}

Message UDPSocket::getMessage() {
  return Message(getRawMessage());
}

void UDPSocket::closeSocket(){
  if(_socketFd > -1){
    close(_socketFd);
  }
}

UDPSocket::~UDPSocket () {
  closeSocket();
}
