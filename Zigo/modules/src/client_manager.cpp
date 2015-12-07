#include "client_manager.h"


ClientManager::ClientManager(size_t cacheSize): _cacheSize(cacheSize), _currentSize(0) {
  if(pthread_mutex_init(&_queueLock, NULL) != 0)
    throw MutexInitializationException();
}

Client* ClientManager::get(const char *id, const char *username, const char *ip, uint16_t port) {
  for(unsigned int i = 0; i < _clientQueue.size(); i++) {
    if (strcmp(_clientQueue[i]->getId(), id) == 0)
      return _clientQueue[i];
  }

  Client *client = new Client(username, ip, port);

  put(client);

  return client;
}

void ClientManager::put(Client *client) {
  if(_currentSize == _cacheSize)
    _clientQueue.erase(_clientQueue.begin());
  _clientQueue.push_back(client);
}

bool ClientManager::contains(char *id) const {
  for(unsigned int i = 0; i < _clientQueue.size(); i++) {
    if (strcmp(_clientQueue[i]->getId(), id) == 0)
      return true;
  }
  return false;
}


int ClientManager::lock() {
  return pthread_mutex_lock(&_queueLock);
}

int ClientManager::unlock() {
  return pthread_mutex_unlock(&_queueLock);
}

ClientManager::~ClientManager() {
  pthread_mutex_destroy(&_queueLock);
}
