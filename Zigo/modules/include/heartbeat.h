#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "client.h"
#include "udp_socket.h"
#include "file.h"
#include "crypto.h"
#include "seeder_node.h"

enum ConnectionState {
  Connected = 0,
  Disconnected = 1
};


class HeartBeat : public Client {
  ConnectionState _state;

public:
  HeartBeat(const char *username, const char *hostname, uint16_t port);
  void run();
  bool reset();
  void stop();

  void queryUsername(char *username);
  void queryId(char *id);
  void queryOnline();

  //Message * execute(Message * _message);
  ~HeartBeat();
};

#endif //HEARTBEAT_H
