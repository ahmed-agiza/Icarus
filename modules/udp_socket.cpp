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

ssize_t UDPSocket::sendRaw(char *data, ssize_t length) {

  return sendto(_socketFd, data, length, 0, (sockaddr *) &_peerAddr, _sinSize);
}

ssize_t UDPSocket::recvRaw(ssize_t length) {
  return recvfrom(_socketFd, _readBuff, length, 0, (sockaddr *) &_peerAddr, &_sinSize);
}

const char *UDPSocket::getReadBuff () const {
  return _readBuff;
}

void UDPSocket::closeSocket(){
  if(_socketFd > -1){
    close(_socketFd);
  }
}

UDPSocket::~UDPSocket () {
  closeSocket();
}
