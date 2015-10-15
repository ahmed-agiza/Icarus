#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "message.h"

#define READ_BUFFER_MAX 2048

class UDPSocket {
protected:
  int _socketFd;
  sockaddr_in _hostAddr;
  sockaddr_in _peerAddr;
//     char * myAddress;
//     char * peerAddress;
  uint16_t _hostPort;
  uint16_t _peerPort;

  char _readBuff[READ_BUFFER_MAX];

  socklen_t _sinSize;

//     bool enabled;
//     pthread_mutex_t mutex;
//
public:
  UDPSocket ();
  int getFd() const;
  const char *getReadBuff () const;
  const char *getRawMessage ();
  Message getMessage();
//     void setFilterAddress (char * _filterAddress);
//     char * getFilterAddress ();

  ssize_t sendRaw(const char *data, ssize_t length) const;
  ssize_t sendRaw(const char *data, ssize_t length, const sockaddr_in &destinationAddr) const;
  ssize_t recvRaw(ssize_t length = READ_BUFFER_MAX);
  ssize_t recvRaw(ssize_t length, sockaddr_in &sourceAddr);
//     int writeToSocket (char * buffer, int maxBytes );
//     int writeToSocketAndWait (char * buffer, int maxBytes,int microSec );
//     int readFromSocketWithNoBlock (char * buffer, int maxBytes );
//     int readFromSocketWithTimeout (char * buffer, int maxBytes, int timeoutSec, int timeoutMilli);
//     int readFromSocketWithBlock (char * buffer, int maxBytes );
//     int readSocketWithNoBlock (char * buffer, int maxBytes );
//     int readSocketWithTimeout (char * buffer, int maxBytes, int timeoutSec, int timeoutMilli);
//     int readSocketWithBlock (char * buffer, int maxBytes );
//     int getMyPort ();
//     int getPeerPort ();
//     void enable();
//     void disable();
//     bool isEnabled();
//     void lock();
//     void unlock();
//     int getSocketHandler();
  void closeSocket();
  ~UDPSocket ();
};
#endif // UDPSOCKET_H
