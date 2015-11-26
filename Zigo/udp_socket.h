#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <time.h>
#include <errno.h>

#define TIMEOUT_RC -10

#include "message.h"
#include "network_exceptions.h"


class UDPSocket {
protected:
  int _socketFd;
  sockaddr_in _hostAddr;
  sockaddr_in _peerAddr;

  uint16_t _hostPort;
  uint16_t _peerPort;

  char _readBuff[MAX_READ_SIZE];
  Message _readMessage;

  socklen_t _sinSize;
  struct timeval _sendTimeout; //the packet sending timeout
  struct timeval _recvTimeout; //the packet receiving timeout
  fd_set _rfds;

  pthread_mutex_t *_lock;

  ssize_t _sendRaw(const char *data, ssize_t length) const; //send raw data to the default address (_peerAddr)
  ssize_t _sendRaw(const char *data, ssize_t length, const sockaddr_in &destinationAddr) const; //send raw data to specific address
  ssize_t _recvRaw(ssize_t length = MAX_READ_SIZE); //receive raw data from the default adress (_peerAddr)
  ssize_t _recvRaw(ssize_t length, sockaddr_in &sourceAddr); //receive raw data from specific adresss
  ssize_t _recvRawTimeout(time_t seconds, suseconds_t mseconds, ssize_t length = MAX_READ_SIZE);

public:
  UDPSocket ();
  UDPSocket (const UDPSocket &other);
  int getFd() const;

  uint16_t initialize(uint16_t port = 0);
  uint16_t initialize(char * peerName, uint16_t port);

  Message getMessage();

  Message recvMessage();

  Message recvMessageTimeout(time_t seconds, suseconds_t mseconds);


  void setSendTimeout(time_t seconds = 0, suseconds_t micro = 0);
  void setRecvTimeout(time_t seconds = 0, suseconds_t micro = 0);
  void setTimeout(time_t seconds = 0, suseconds_t micro = 0); // a function for both receiving and sending timeout

  void setMutex(pthread_mutex_t *mutex);
  void lock();
  void unlock();

  char *getPeerName() const;
  void setPort(uint16_t port) ;
  int getPortNumber() const;

  sockaddr_in getPeerAddress() const;

  void setPeerAddress(sockaddr_in peer);

  ssize_t sendMessage(Message message);

  void closeSocket();
  ~UDPSocket ();
};
#endif // UDPSOCKET_H
