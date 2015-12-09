#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <vector>
using std::vector;

#include "client.h"

#define CACHE_SIZE 30


class ClientManager {
  vector<Client *> _clientQueue;
  size_t _cacheSize;
  size_t _currentSize;

  pthread_mutex_t _queueLock;
public:
  ClientManager(size_t cacheSize = CACHE_SIZE);

  Client* get(const char *id, const char *username = NULL, const char *ip = NULL, uint16_t port = -1);
  void put(Client *client);
  bool contains(char *id) const;


  int lock();
  int unlock();
  File *openFile();

  ~ClientManager();
};


#endif
