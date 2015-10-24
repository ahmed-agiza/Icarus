#include "udp_socket.h"

UDPSocket::UDPSocket () : _lock(0) {
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
  return recvfrom(_socketFd, _readBuff, length + 1, 0, (sockaddr *) &sourceAddr, &_sinSize);
}

const char *UDPSocket::getReadBuff () const {
  return _readBuff;
}

const char *UDPSocket::getRawMessage () {
  ssize_t recvBytes = recvRaw();
  if(recvBytes < 0) {
     if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
        throw "Connection timeout!";
    fprintf(stderr, "%s\n", "An error occured while receiving the message.");
  }
  char *recvData = new char[recvBytes + 1];
  strcpy(recvData, getReadBuff());
  recvData[recvBytes] = 0;
  return recvData;
}

void UDPSocket::setSendTimeout(time_t seconds, suseconds_t micro) {
  _sendTimeout.tv_sec = seconds;
  _sendTimeout.tv_usec = micro;
  if (setsockopt (_socketFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&_sendTimeout, sizeof(_sendTimeout)) < 0)
      throw "Failed to set send timeout.";
}

void UDPSocket::setRecvTimeout(time_t seconds, suseconds_t micro) {
  _recvTimeout.tv_sec = seconds;
  _recvTimeout.tv_usec = micro;
  if (setsockopt (_socketFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&_recvTimeout, sizeof(_recvTimeout)) < 0)
      throw "Failed to set receive timeout.";
}

void UDPSocket::setTimeout(time_t seconds, suseconds_t micro) {
  setSendTimeout(seconds, micro);
  setRecvTimeout(seconds, micro);
}

Message UDPSocket::getMessage() {
  return Message(getRawMessage());
}

void UDPSocket::setMutex(pthread_mutex_t *mutex) {
  _lock = mutex;
}
void UDPSocket::lock() {
  if(_lock)
    pthread_mutex_lock(_lock);
}
void UDPSocket::unlock() {
  if(_lock)
    pthread_mutex_unlock(_lock);
}

void UDPSocket::closeSocket(){
  if(_socketFd > -1){
    close(_socketFd);
  }
}

UDPSocket::~UDPSocket () {
  closeSocket();
}
