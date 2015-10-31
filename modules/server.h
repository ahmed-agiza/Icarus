#ifndef SERVER_H
#define SERVER_H

#define __USE_GNU
#include <search.h>

#include "server_socket.h"
#include "job.h"


class Server {
private:
  ServerSocket * _serverSocket;
  uint16_t _listenPort;
  const char *_getRawRequest();
  const char *_getRawRequestTimeout(time_t seconds = 0, suseconds_t mseconds = 0);
  Message _getMessage();
  void _sendReply();
  ssize_t _acknowledge(const char *request);
  bool _acknowledgeAndWait(const char *request);
  //Message * getRequest();
  //Message * doOperation();
  //void sendReply (Message * _message);
  pthread_mutex_t _terminationLock;
  bool _terminated; //used to terminate the server when a client wishes to close connection

  struct hsearch_data _clientsTable; //will store address of clients and ports used

  int addClient(char *addr, int port, Job *job = 0);
  int getClientPort(char *addr);
  int removeClient(char *addr);

  //A client tree structure.
  class ClientNode {
    ClientNode* _next;
    ClientNode* _prev;
    char _clientAddr[64];
    char *_nodeKey;
    uint16_t _port;
    Job *_reponderJob;
    friend class Server;
  public:
    ClientNode(const char *addr):_next(0),_prev(0),_nodeKey(0){
      strcpy(_clientAddr, addr);
    }

    void setPort(uint16_t port) {
      _port = port;
    }

    uint16_t getPort() {
      return _port;
    }

    void setNodeKey(char *nodeKey) {
      _nodeKey = nodeKey;
    }

    char *getNodeKey() {
      return _nodeKey;
    }

    void setJob(Job *job) {
      _reponderJob = job;
    }

    Job *getJob(){
      return _reponderJob;
    }

    ClientNode *addNext(ClientNode *next) {
      next->_prev = this;
      if(_next){
        next->_next = _next;
        _next->_prev = next;
        _next = next;
      } else
        _next = next;
      return _next;
    }

    ClientNode *addPrev(ClientNode *prev) {
      prev->_next = this;
      if(_prev){
        prev->_prev = _prev;
        _prev->_next = prev;
        _prev = prev;
      } else
        _prev = prev;
      return _prev;
    }

    ClientNode *remove(){
      if(_next){
        _next->_prev = _prev;
      }
      if(_prev){
        _prev->_next = _next;
      }
      return this;
    }

    ~ClientNode(){
      if(_nodeKey)
        delete _nodeKey;
    }

  };

  ClientNode *_clientHead;
  ClientNode *_clientTail;

public:
  Server(uint16_t listenPort);
  void listen();

  void serveRequest(const char *request);
  ~Server();



};
#endif // SERVER_H
