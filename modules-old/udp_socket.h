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

  char _readBuff[READ_BUFFER_MAX]; // buffer used for receiving messages of size 2048 bytes

  socklen_t _sinSize;
  struct timeval _sendTimeout; //the packet sending timeout
  struct timeval _recvTimeout; //the packet receiving timeout
  fd_set _rfds;

//     bool enabled;
  pthread_mutex_t *_lock;
//
public:
  UDPSocket ();
  int getFd() const;
  const char *getReadBuff () const;
  const char *getRawMessage ();
  const char *getRawMessageTimeout (time_t seconds, suseconds_t mseconds);
  Message getMessage();
  Message getMessageTimeout(time_t seconds, suseconds_t mseconds);
//     void setFilterAddress (char * _filterAddress);
//     char * getFilterAddress ();

  ssize_t sendRaw(const char *data, ssize_t length) const; //send raw data to the default address (_peerAddr)
  ssize_t sendRaw(const char *data, ssize_t length, const sockaddr_in &destinationAddr) const; //send raw data to specific address
  ssize_t recvRaw(ssize_t length = READ_BUFFER_MAX); //receive raw data from the default adress (_peerAddr)
  ssize_t recvRaw(ssize_t length, sockaddr_in &sourceAddr); //receive raw data from specific adresss
  ssize_t recvRawTimeout(time_t seconds, suseconds_t mseconds, ssize_t length = READ_BUFFER_MAX);


  void setSendTimeout(time_t seconds = 0, suseconds_t micro = 0);
  void setRecvTimeout(time_t seconds = 0, suseconds_t micro = 0);
  void setTimeout(time_t seconds = 0, suseconds_t micro = 0); // a function for both receiving and sending timeout

  void setMutex(pthread_mutex_t *mutex);
  void lock();
  void unlock();

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

//     int getSocketHandler();
  void closeSocket();
  ~UDPSocket ();
};
#endif // UDPSOCKET_H
