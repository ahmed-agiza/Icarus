#ifndef UDPCLIENTSOCKET_H
#define UDPCLIENTSOCKET_H

#include "udp_socket.h"

class ClientSocket : public UDPSocket {
  public:
    ClientSocket ();
    bool initializeClient (char * peerName, uint16_t peerPort); //initialize client with the peerName and port
    ~ClientSocket();
};
#endif // UDPCLIENTSOCKET_H
