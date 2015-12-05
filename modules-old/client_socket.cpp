#include "client_socket.h"

ClientSocket::ClientSocket ():UDPSocket(){
}

bool ClientSocket::initializeClient (char * peerName, uint16_t peerPort){
  _peerAddr.sin_family = AF_INET;
  _peerAddr.sin_port = htons(peerPort);
  hostent *serverName = gethostbyname(peerName);
  if(serverName == NULL)
    throw "Failed to resolve hostname.\n";
  memcpy((char *)&_peerAddr.sin_addr.s_addr, (char*) serverName->h_addr, serverName->h_length);
  memset(&(_peerAddr.sin_zero), 0, 8);
  return 1;
}

void ClientSocket::setPort(uint16_t port) {
    _peerAddr.sin_port = htons(port);
}


ClientSocket::~ClientSocket (){

}
