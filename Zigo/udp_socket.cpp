#include "udp_socket.h"

UDPSocket::UDPSocket () : _lock(0) {
  _socketFd = socket(AF_INET, SOCK_DGRAM, 0);
  if(_socketFd < 0)
  throw SocketException();
  _sinSize = sizeof(sockaddr_in);
}

int UDPSocket::getFd() const {
  return _socketFd;
}

uint16_t UDPSocket::initialize(uint16_t port) {
  _hostAddr.sin_family = AF_INET;
  _hostAddr.sin_port = htons(port);  //generate a random available port
  _hostAddr.sin_addr.s_addr = INADDR_ANY; //inet_addr(peerName); //receive messages from this address only
  bzero(&_hostAddr.sin_zero, 8);
  if(bind(_socketFd, (sockaddr *) &_hostAddr, sizeof(_hostAddr)) == -1)
  throw ServerBindingException();

  if(getsockname(_socketFd, (sockaddr *) &_hostAddr, &_sinSize) == -1)
  throw ClientBindingException();

  return ntohs(_hostAddr.sin_port); //will be sent to the client to start communication at this port
}


uint16_t UDPSocket::initialize(char * peerName, uint16_t port) {
  _peerAddr.sin_family = AF_INET;
  _peerAddr.sin_port = htons(port);
  hostent *serverName = gethostbyname(peerName);
  if(serverName == NULL)
  throw HostResolveException();
  memcpy((char *)&_peerAddr.sin_addr.s_addr, (char*) serverName->h_addr, serverName->h_length);
  memset(&(_peerAddr.sin_zero), 0, 8);
  return ntohs(_peerAddr.sin_port);
}

ssize_t UDPSocket::_sendRaw(const char *data, ssize_t length) const {
  return _sendRaw(data, length, _peerAddr);
}

ssize_t UDPSocket::_sendRaw(const char *data, ssize_t length, const sockaddr_in &destinationAddr) const {
  return sendto(_socketFd, data, length + 1, 0, (sockaddr *) &destinationAddr, _sinSize);
}

ssize_t UDPSocket::_recvRaw(ssize_t length) {
  return _recvRaw(length, _peerAddr);
}

ssize_t UDPSocket::_recvRaw(ssize_t length, sockaddr_in &sourceAddr) {
  ssize_t readLength = recvfrom(_socketFd, _readBuff, length + 1, 0, (sockaddr *) &sourceAddr, &_sinSize);
  _readMessage = Message(_readBuff);
  return readLength;
}

ssize_t UDPSocket::_recvRawTimeout(time_t seconds, suseconds_t mseconds, ssize_t length) {
  FD_ZERO(&_rfds);
  FD_SET(_socketFd, &_rfds);

  _recvTimeout.tv_sec = seconds;
  _recvTimeout.tv_usec = mseconds;
  if (select(_socketFd + 1, &_rfds, NULL, NULL, &_recvTimeout) < 0) {
    perror("Select error");
    throw ReceiveFailureException();
    exit(1);
  }

  if (FD_ISSET(_socketFd, &_rfds)) {
    ssize_t readLength = recvfrom(_socketFd, _readBuff, length + 1, 0, (sockaddr *) &_peerAddr, &_sinSize);
    _readMessage = Message(_readBuff);
    return readLength;
  } else {
    throw ReceiveTimeoutException();
    return TIMEOUT_RC;
  }
}

Message UDPSocket::getMessage() {
  return _readMessage;
}

Message UDPSocket::recvMessage() {
  ssize_t readLength = _recvRaw();
  if (readLength < 0)
    throw ReceiveFailureException();
  return _readMessage;
}

Message UDPSocket::recvMessageTimeout(time_t seconds, suseconds_t mseconds) {
  ssize_t readLength = _recvRawTimeout(seconds, mseconds);
  if (readLength < 0)
  throw ReceiveFailureException();
  return _readMessage;
}



void UDPSocket::setSendTimeout(time_t seconds, suseconds_t micro) {
  _sendTimeout.tv_sec = seconds;
  _sendTimeout.tv_usec = micro;
  if (setsockopt (_socketFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&_sendTimeout, sizeof(_sendTimeout)) < 0)
  throw SetSendTimeoutException();
}

void UDPSocket::setRecvTimeout(time_t seconds, suseconds_t micro) {
  _recvTimeout.tv_sec = seconds;
  _recvTimeout.tv_usec = micro;
  if (setsockopt (_socketFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&_recvTimeout, sizeof(_recvTimeout)) < 0)
  throw SetReceiveTimeoutException();
}

void UDPSocket::setTimeout(time_t seconds, suseconds_t micro) {
  setSendTimeout(seconds, micro);
  setRecvTimeout(seconds, micro);
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


void UDPSocket::setPort(uint16_t port) {
  _peerAddr.sin_port = htons(port);
}

char *UDPSocket::getPeerName() const {
  return (char *)inet_ntoa(_peerAddr.sin_addr);
}

int UDPSocket::getPortNumber() const {
  return _peerAddr.sin_port;
}

sockaddr_in UDPSocket::getPeerAddress() const {
  return _peerAddr;
}

void UDPSocket::setPeerAddress(sockaddr_in peer) {
  _peerAddr = peer;
}


ssize_t UDPSocket::sendMessage(Message message){
  const char *bytes = message.getBytes();
  ssize_t sentBytes = _sendRaw(bytes, strlen(bytes));
  delete bytes;
  return sentBytes;
}

UDPSocket::~UDPSocket () {
  closeSocket();
}
