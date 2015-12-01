#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <map>
using std::vector;
using std::map;

#include "udp_socket.h"
#include "job.h"
#include "thread_pool.h"
#include "file.h"

struct StringCompare {
   bool operator()(char const *a, char const *b) {
      return (strcmp(a, b) != 0);
   }
};


class Server {
private:
  UDPSocket * _serverSocket;
  uint16_t _listenPort;

  pthread_mutex_t _terminationLock;
  bool _terminated; //used to terminate the server when a client wishes to close connection


  map<char *, ClientNode *, StringCompare> _clients;

  size_t _jobCount;
  ThreadPool<Job> _jobsPool;


  Message _getMessage();
  Message _getMessageTimeout(time_t seconds = 0, suseconds_t mseconds = 0);

  void _sendReply();
  ssize_t _sendMessage(Message message);

  ClientNode *_addClient(char *id, int port, Job *job = 0);
  int _getClientPort(char *id);
  ClientNode *_getClient(char *id);
  int _removeClient(char *id);


public:
  Server(uint16_t listenPort);
  void listen();

  size_t getJobCount() const;

  void serveRequest(Message &request);

  ~Server();



};
#endif // SERVER_H
