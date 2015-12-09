#ifndef SEEDER_H
#define SEEDER_H

#include <vector>
#include <map>
using std::vector;
using std::map;

#include "udp_socket.h"
#include "seeder_job.h"
#include "thread_pool.h"
#include "file.h"

typedef map<char *, SeederNode *, StringCompare> SeedersMap;

class Seeder {
private:
  UDPSocket * _seederSocket;
  uint16_t _listenPort;

  pthread_mutex_t _mapLock;

  SeedersMap _clients;
  vector<SeederNode *> _recentClients;

  SharedPair _shared;
  size_t _jobCount;
  ThreadPool<SeederJob> _jobsPool;

  char _id[128];

  Message _getMessage();
  Message _getMessageTimeout(time_t seconds = 0, suseconds_t mseconds = 0);

  void _sendReply();
  ssize_t _sendMessage(Message message);

  SeederNode *_addClient(char *id, int port, SeederJob *job = 0);
  int _getClientPort(char *id);
  SeederNode *_getClient(char *id);
  int _removeClient(char *id);

  static void *_threadDoneWrapper(Thread *, void *);
  void _threadDoneCallback(SeederJob *);

public:
  Seeder(uint16_t listenPort);
  void listen();

  size_t getJobCount() const;

  void serveRequest(Message &request);

  ~Seeder();
};
#endif //SEEDER_H
