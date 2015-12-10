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
#include "crypto.h"
#include "thread.h"

#ifndef STRING_COMPARE_OPERATOR
#define STRING_COMPARE_OPERATOR
struct StringCompare {
   bool operator()(char const *a, char const *b) {
      return (strcmp(a, b) < 0);
   }
};
#endif

class Server : public Thread {
protected:
  UDPSocket * _serverSocket;
  uint16_t _listenPort;

  pthread_mutex_t _terminationLock;
  bool _terminated; //used to terminate the server when a client wishes to close connection

  char _publicRSA[2048];
  char _privateRSA[2048];
  char _stegKey[2048];
  char _id[128];

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
  static void *_threadDoneWrapper(Thread *, void *);
  void _threadDoneCallback(Job *);
public:
  Server(uint16_t listenPort);
  void listen();

  void run();
  bool reset();
  void stop();

  size_t getJobCount() const;

  void serveRequest(Message &request);

  const char *getStegKey();
  const char *getPublicRSA();
  const char *getPrivateRSA();

  void setStegKey(char *key);

  ~Server();



};
#endif // SERVER_H
