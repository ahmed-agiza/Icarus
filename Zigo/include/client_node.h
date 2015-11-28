#ifndef CLIENT_NODE_H
#define CLIENT_NODE_H

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

#endif
