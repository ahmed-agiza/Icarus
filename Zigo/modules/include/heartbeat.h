#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "client.h"
#include "udp_socket.h"
#include "file.h"
#include "crypto.h"
#include "seeder_node.h"

enum ConnectionState {
  Connecting = 0,
  Connected = 1,
  Disconnected = 2
};

#define PINGING_TIME 3


class HeartBeat : public Client {
  ConnectionState _state;

  pthread_condattr_t _timerAttr;
  pthread_cond_t _timerCv;
  pthread_mutex_t _timerMutex;

  timespec _pingTime;
  ThreadCallback _connectedCallback;
  void *_connectionParent;

  char _searchParam[128];

  void _waitTimer(long waitVal);
  void _wakeTimer();

public:
  HeartBeat(const char *username, const char *hostname, uint16_t port, uint16_t serverPort);
  void run();
  bool reset();
  void stop();

  void queryUsername(char *username);
  void queryId(char *id);
  void queryOnline();
  void queryRecent();

  const char *getSearchParam();

  bool isConnected() const;
  bool isConnecting() const;

  void setConnectedCallback(ThreadCallback callback, void *parent);

  //Message * execute(Message * _message);
  ~HeartBeat();
};

#endif //HEARTBEAT_H
