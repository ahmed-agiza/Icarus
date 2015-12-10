#ifndef UDPSERVERSOCKET_H
#define UDPSERVERSOCKET_H

#include "udp_socket.h"

class ServerSocket : public UDPSocket {
public:
  ServerSocket ();
  bool initializeServer (uint16_t hostPort); //used to init the socket and bind it with hostPort.

  //returns the port that will be sent to the client to connect after spawing a thread to handle the client.
  uint16_t initializeServer (const char *peerName);
  sockaddr_in getClientAddress() const;
  void setClientAddress(sockaddr_in client);
  char *getPeerName() const;
  int getPortNumber() const;
   ~ServerSocket ();
};
#endif // UDPSERVERSOCKET_H
