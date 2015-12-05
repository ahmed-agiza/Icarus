#ifndef CLIENT_NODE_H
#define CLIENT_NODE_H

#include <map>
using std::map;

#include "udp_socket.h"
#include "file.h"

class Job;

//A client tree structure.
class ClientNode {
  char _clientId[128];
  char _username[128];
  UDPSocket *_socket;
  uint16_t _port;
  Job *_reponderJob;
  map<int, File *> _openFiles;
public:

  ClientNode(const char *id);

  ClientNode(const char *id, UDPSocket *socket);

  ClientNode(const ClientNode &other);

  void setClientId(const char *id);

  const char *getClientId() const;

  UDPSocket *getSocket();

  void setSocket(UDPSocket *socket);

  void setPort(uint16_t port);

  uint16_t getPort();


  void setJob(Job *job);

  void setUsername(const char *username);

  Job *getJob();

  File *addFile(int fd, File * file);

  bool removeFile(int fd);

  File *getFile(int fd);

  bool hasOpenFile(int fd);

  map<int, File *> &getFileTable();

  ~ClientNode();

};

#endif
