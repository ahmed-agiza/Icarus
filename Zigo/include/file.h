#ifndef FILE_H
#define FILE_H

#include "message.h"
#include "udp_socket.h"

class File {
  int _fd;
  char _fileId[1024];
  bool _isLocal;
  off_t _offset;
  UDPSocket *_socket;
  bool _isOpen;
  bool _isEOF;
  File();
public:
  File(const File &other);
  static File *open(const char *pathname, int flags);
  static File *open(const char *pathname, int flags, mode_t mode);
  static File *ropen(UDPSocket *socket, char *fileId);
  File duplicate(const File &other);
  const char *getFileId() const;
  bool isLocal() const;
  bool isOpen() const;
  ssize_t read(void *buf, size_t count);
  ssize_t write(const void *buf, size_t count);
  void setOffset(off_t offset);
  off_t getOffset() const;
  int close();
  int getFd();
  bool isEOF() const;
  ~File();
};


#endif
