#ifndef FILE_H
#define FILE_H

#include "message.h"
#include "udp_socket.h"

#include <linux/limits.h>
#include <fcntl.h>

class File {
  int _fd;
  char _fileId[1024];
  char _userId[128];
  bool _isLocal;
  off_t _offset;
  UDPSocket *_socket;
  bool _isOpen;
  bool _isEOF;
  char _filePath[PATH_MAX];
  File();
public:
  File(const File &other);
  static File *open(const char *pathname, int flags);
  static File *open(const char *pathname, int flags, mode_t mode);
  static File *ropen(UDPSocket *socket, char *fileId, const char *userId);

  static bool exists(const char *filename);
  static int remove(const char *filePath);
  static void mkdirRecursive(const char *dir);
  static void createDirIfNotExists(const char *dir);
  static void joinPaths(char* destination, const char* path1, const char* path2);
  static void getWorkingDirectory(char *buf, size_t bufSize);

  File duplicate(const File &other);
  const char *getFileId() const;
  bool isLocal() const;
  bool isOpen() const;
  int remove();
  const char *getFilePath() const;
  ssize_t read(void *buf, size_t count);
  ssize_t write(const void *buf, size_t count);
  void setOffset(off_t offset);
  void setUserId(const char *userId);
  off_t getOffset() const;
  int close();
  int getFd();
  bool isEOF() const;
  ~File();
};


#endif
