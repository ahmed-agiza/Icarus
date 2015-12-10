#ifndef FILE_H
#define FILE_H

#include "message.h"
#include "udp_socket.h"

#include <linux/limits.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>


#define DIR_STORAGE "./storage/"
#define DIR_BUFFER  "./buffer/"


#include <vector>
using std::vector;

enum FileState {
  AlreadyExists = 0,
  Locked = 1,
  Created = 2,
  Opened = 3,
  Noent = 4,
  AlreadyClosed = 5,
  FailedOperation = 6,
  Closed = 7
};

enum FileType {
  Buffer = 0,
  Receive = 1,
  Send = 2
};

enum FileMode {
  ReadOnly = 0,
  AttemptCreate = 1,
  ForceCreate = 2,
  Update = 3
};

struct FileInfo {
  char filePath[2048];
  char fileId[128];
  char userId[128];
  char recepientId[128];
  FileType fileType;
};

class File {
  int _fd;
  char _fileId[1024];
  char _userId[128];
  bool _isLocal;
  bool _isLocked;
  bool _isLockOwner;
  char _lockOwner[128];
  off_t _offset;
  UDPSocket *_socket;
  bool _isOpen;
  bool _isEOF;
  char _filePath[PATH_MAX];
  long _fileSize;
  File();
public:
  File(const File &other);
  static File *open(const char *pathname, int flags);
  static File *open(const char *pathname, int flags, mode_t mode);
  static File *ropen(UDPSocket *socket, char *fileId, const char *userId, char *messageId = DEFAULT_MESSAGE_ID, FileMode mode = ReadOnly, FileState *state = NULL, long *currentFileSize = NULL);

  static bool exists(const char *filename);
  static int remove(const char *filePath);
  static void mkdirRecursive(const char *dir);
  static void createDirIfNotExists(const char *dir);
  static void joinPaths(char* destination, const char* path1, const char* path2);
  static void getWorkingDirectory(char *buf, size_t bufSize);


  void setLockOwner(const char *owner);
  bool isLockOwner(const char *owner) const;
  int forceUnlock();

  int rename(const char *newname);
  static int rename(const char *originalName, const char *newname);
  static bool isLocked(const char *filename);

  File duplicate(const File &other);
  const char *getFileId() const;
  bool isLocal() const;
  bool isOpen() const;
  bool isLocked() const;
  bool isLockOwner() const;
  int lock();
  int unlock();
  int remove();
  const char *getFilePath() const;
  long getFileSize() const;
  ssize_t read(void *buf, size_t count, char *messageId = DEFAULT_MESSAGE_ID);
  ssize_t write(const void *buf, size_t count, char *messageId = DEFAULT_MESSAGE_ID, bool rsaEncrypted = false, size_t encodeLen = -1);
  void setOffset(off_t offset, char *messageId = DEFAULT_MESSAGE_ID);
  void setUserId(const char *userId);
  off_t getOffset() const;
  int close();
  int getFd();
  bool isEOF() const;
  static vector<FileInfo> getAllFiles(FileType type);
  static void getUserFiles(char* userId, FileType type, vector<FileInfo>& vecFiles);
  ~File();
};


#endif
