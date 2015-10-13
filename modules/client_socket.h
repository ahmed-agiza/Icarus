#ifndef UDPCLIENTSOCKET_H
#define UDPCLIENTSOCKET_H

#include "udp_socket.h"


class ClientSocket : public UDPSocket {
  public:
    ClientSocket ();
    bool initializeClient (char * peerName, uint16_t peerPort);
    ~ClientSocket();
};
#endif // UDPCLIENTSOCKET_H
