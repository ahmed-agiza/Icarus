#ifndef UDPSERVERSOCKET_H
#define UDPSERVERSOCKET_H

#include "udp_socket.h"

class ServerSocket : public UDPSocket {
public:
  ServerSocket ();
  bool initializeServer (uint16_t hostPort);
  uint16_t initializeServer (const char *peerName);
  sockaddr_in getClientAddress() const;
  char *getPeerName() const;
  int getPortNumber() const;
   ~ServerSocket ();
};
#endif // UDPSERVERSOCKET_H
