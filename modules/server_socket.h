#ifndef UDPSERVERSOCKET_H
#define UDPSERVERSOCKET_H

#include "udp_socket.h"

class ServerSocket : public UDPSocket {
public:
  ServerSocket ();
  bool initializeServer (uint16_t hostPort);
   ~ServerSocket ();
};
#endif // UDPSERVERSOCKET_H
