#include "file.h"

File::File() {
  _fd = -1;
  _fileSize = 0;
}

File *File::open(const char *pathname, int flags) {
  File *file = new File;
  memset(file->_filePath, 0, PATH_MAX);
  strcpy(file->_filePath, pathname);
  file->_isLocal = true;
  file->_socket = 0;
  file->_isOpen = true;
  file->_offset = 0;
  file->_fd = ::open(pathname, flags);
  if (file->_fd > 0)
    file->_fileSize = lseek(file->_fd, 0, SEEK_END);
  lseek(file->_fd, 0, SEEK_SET);
  file->_isEOF = false;
  file->_isLockOwner = false;
  return file;
}

File *File::open(const char *pathname, int flags, mode_t mode) {
  File *file = new File;
  memset(file->_filePath, 0, PATH_MAX);
  strcpy(file->_filePath, pathname);
  file->_isLocal = true;
  file->_socket = 0;
  file->_fd = ::open(pathname, flags, mode);
  if (file->_fd > 0)
    file->_fileSize = lseek(file->_fd, 0, SEEK_CUR);
  lseek(file->_fd, 0, SEEK_SET);
  file->_isOpen = true;
  file->_offset = 0;
  file->_isEOF = false;
  file->_isLockOwner = false;
  return file;
}
File *File::ropen(UDPSocket *socket, char *fileId, const char *userId,
                  char *messageId, FileMode mode, FileState *state, long *currentFileSize) {

  File *file = new File;
  file->_socket = socket;
  file->_isLockOwner = false;
  strcpy(file->_userId, userId);
  char messageBody[PATH_MAX + 130];
  memset(messageBody, 0, PATH_MAX + 130);
  sprintf(messageBody, "%s;%d;", fileId, (int) mode);
  Message openMessage(Open, messageBody, file->_userId, messageId);
  ssize_t sentBytes = file->_socket->sendMessage(openMessage);
  (void) sentBytes;

  uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
  Message replyMessage = file->_socket->recvMessageTimeout(clientReplyTo, 0);

  if (replyMessage.getType() != Reply) {
    char invalidReplyMessage[LOG_MESSAGE_LENGTH];
    sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
    Logger::error(invalidReplyMessage);
    throw InvalidReplyException();
  }

  int fdReply, stateReply; long currentSize;

  if (sscanf(replyMessage.getBody(), "%d;%d;%ld;", &fdReply, &stateReply, &currentSize) != 3) {
    printf("--Invalid reply: %s\n", replyMessage.getBody());
    throw InvalidReplyException();
  }

  if(currentFileSize)
    *currentFileSize = currentSize;

  file->_offset = currentSize;

  strcpy(file->_fileId, fileId);

  file->_fd = fdReply;
  if (stateReply == Opened || stateReply == Created) {
    file->_isOpen = true;
    file->_isLocal = false;
    file->_isEOF = false;
  } else {
    file->_isOpen = false;
    file->_isLocal = false;
    file->_isEOF = false;
  }

  if(state)
    *state = (FileState)stateReply;

  Message ackReply(Acknowledge, "1", file->_userId, messageId);
  file->_socket->sendMessage(ackReply);

  return file;
}

void File::setUserId(const char *userId) {
  strcpy(_userId, userId);
}

bool File::exists(const char *filename){
  FILE *file;
  if ((file = fopen(filename, "r"))){
      fclose(file);
      return 1;
  }
  return 0;
}

int File::remove(const char *filePath) {
  return ::remove(filePath);
}

bool File::isLocked(const char *filename) {
  int fd = ::open(filename, O_RDONLY);
  if(!flock(fd, LOCK_EX|LOCK_NB)) {
    ::close(fd);
    return false;
  }

  flock(fd, LOCK_UN);
  ::close(fd);
  return true;
}

void File::setLockOwner(const char *owner) {
  strcpy(_lockOwner, owner);
}

bool File::isLockOwner(const char *owner) const{
  return (strcmp(_lockOwner, owner) == 0);
}

int File::forceUnlock() {
  _isLocked = false;
  return flock(_fd, LOCK_UN);
}


void File::joinPaths(char* destination, const char* path1, const char* path2) {
    if(path1 == NULL && path2 == NULL) {
        strcpy(destination, "");;
    }
    else if(path2 == NULL || strlen(path2) == 0) {
        strcpy(destination, path1);
    }
    else if(path1 == NULL || strlen(path1) == 0) {
        strcpy(destination, path2);
    }
    else {
        char directory_separator[] = "/";

        const char *last_char = path1;
        while(*last_char != '\0')
            last_char++;
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}

void File::createDirIfNotExists(const char *dir) {
  struct stat st = {0};

  if (stat(dir, &st) == -1) {
      mkdirRecursive(dir);
  }

}

void File::mkdirRecursive(const char *dir) {
  char tmp[256];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp),"%s",dir);
  len = strlen(tmp);
  if(tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for(p = tmp + 1; *p; p++)
  if(*p == '/') {
    *p = 0;
    mkdir(tmp, S_IRWXU);
    *p = '/';
  }
  mkdir(tmp, S_IRWXU);
}

File::File(const File &other) {
  if (other.isLocal()) {
    _fd = other._fd;
    _isLocal = true;
  } else {
    _fd = other._fd;
    _isLocal = false;
  }
  _isOpen = other._isOpen;
  _offset = other._offset;
}

File File::duplicate(const File &other) {
  File file;
  if (other.isLocal()) {
    file._fd = dup(other._fd);
    file._isLocal = true;
  } else {
    file._fd = other._fd;
    file._isLocal = false;
  }
  file._isOpen = other._isOpen;
  file._offset = other._offset;
  return file;
}

void File::getWorkingDirectory(char *buf, size_t bufSize) {
  getcwd(buf, bufSize);
}

bool File::isOpen() const {
  return _isOpen;
}

bool File::isLocal() const {
  return _isLocal;
}

bool File::isEOF() const {
  return _isEOF;
}

ssize_t File::read(void *buf, size_t count, char *messageId) {
  if (isLocal()) {
    if (!isEOF()){
      lseek(_fd, SEEK_SET, _offset);
      ssize_t readBytes = ::read(_fd, buf, count);
      _offset += readBytes;
      if (readBytes == 0)
        _isEOF = true;
      return readBytes;
    }
    return 0;
  } else {
    char params[32];
    sprintf(params, "%d\n%zd", _fd, count);
    Message readMessage(Read, params, _userId, messageId);
    printf("Params: %s\n", params);
    ssize_t sentBytes = _socket->sendMessage(readMessage);
    (void) sentBytes;
    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

    Message replyMessage = _socket->recvMessageTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply && replyMessage.getType() != Eof) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    if(replyMessage.getType() == Eof) {
      printf("isEOF! %d\n", (int) _offset);
      _isEOF = true;
      *((char *)buf) = 0;
    } else {
      strcpy((char *)buf, replyMessage.getBody());
      printf("(read) Read Size: %d\nCurrent offset: %d\n", (int)strlen((char *)buf), (int)_offset);
      _offset += strlen((char *)buf);
    }

    char seekOff[32];
    sprintf(seekOff, "%d", (int)_offset);
    Message seekMessage(Lseek, seekOff, _userId, DEFAULT_MESSAGE_ID);
    ssize_t sentOffset = _socket->sendMessage(seekMessage);
    return sentOffset;
  }
}


ssize_t File::write(const void *buf, size_t count, char *messageId, bool rsaEncrypted, size_t encodeLen) {
  if (isLocal()) {
    lseek(_fd, SEEK_SET, _offset);
    ssize_t writtenByes = ::write(_fd, buf, count);
    _offset += writtenByes;
    return writtenByes;
  } else {
    char params[5860];
    char encodedBuffer[5000];

    if ((int)encodeLen < 0) {

      Crypto::base64Encode(buf, count, encodedBuffer, 5000);
    } else
      Crypto::base64Encode(buf, encodeLen, encodedBuffer, 5000);
    if (strlen(encodedBuffer) > 0)
      sprintf(params, "%d\n%zd\n%s", _fd, count, (char *)encodedBuffer);
    else
      sprintf(params, "%d\n0\n0", _fd);
    Message writeMessage;
    writeMessage = Message(Write, params, _userId, messageId);
    if (rsaEncrypted)
      writeMessage.setEncoding(RSAEncryption);
    ssize_t sentBytes = _socket->sendMessage(writeMessage);
    (void) sentBytes;
    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

    Message replyMessage = _socket->recvMessageTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }

    int writtenBytes = atoi(replyMessage.getBody());
    _offset += writtenBytes;
    return writtenBytes;
  }
  return -1;
}

long File::getFileSize() const {
  return _fileSize;
}

const char *File::getFileId() const {
  return _fileId;
}

bool File::isLockOwner() const {
  return _isLockOwner;
}

void File::setOffset(off_t offset, char *messageId) {
  if (isLocal()) {
    _offset = offset;
    _isEOF = false;
  } else {
    char seekOff[32];
    memset(seekOff, 0, 32);
    sprintf(seekOff, "%d\n%ld", _fd, (long)offset);
    Message seekMessage(Lseek, seekOff, _userId, messageId);
    ssize_t sentBytes = _socket->sendMessage(seekMessage);
    (void) sentBytes;
    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();

    Message replyMessage = _socket->recvMessageTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    _offset = (off_t)atol(replyMessage.getBody());
  }
}

off_t File::getOffset() const {
  return _offset;
}

int File::close() {
  if (isLocal()) {
    if (_fd >= 0){
      _isOpen = false;
      unlock();
      return ::close(_fd);
    } else
      return -1;
  } else {
    char fd[32];
    int state;
    memset(fd, 0, 32);
    sprintf(fd, "%d", _fd);
    printf("Closing remote file %s\nMessage closeMessage(Close, fd)\n", fd);
    fflush(stdout);
    Message closeMessage(Close, fd, _userId, DEFAULT_MESSAGE_ID);
    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
    ssize_t sentBytes = _socket->sendMessage(closeMessage);
    (void) sentBytes;

    Message replyMessage = _socket->recvMessageTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply || sscanf(replyMessage.getBody(), "%*d;%d;", &state) != 1) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    if ((FileState)state != Closed && (FileState)state != AlreadyClosed) {
      fprintf(stderr, "Failed to close the file %d\n", state);
    } else
      _isOpen = false;
    return (int)state;
  }
  return -1;
}

int File::remove() {
  if (isLocal()){
    const char *filePath = getFilePath();
    return remove(filePath);
  }
  return -1;
}

int File::rename(const char *newname) {
  if(isLocal()) {
    return rename(getFilePath(), newname);
  }
  return -1;
}

int File::rename(const char *originalName, const char *newname) {
  return ::rename(originalName, newname);
}

bool File::isLocked() const {
  if(isLocal()) {
    return _isLocked;
  } else
    return false;
}

int File::lock() {
  if(isLocal()) {
    _isLocked = true;
    _isLockOwner = true;
    return flock(_fd, LOCK_EX);
  }
  return -1;
}

int File::unlock() {
  if(isLocal() && _isLockOwner) {
    _isLocked = false;
    return flock(_fd, LOCK_UN);
  }
  return -1;
}

const char *File::getFilePath() const {
  if (isLocal()) {
    return _filePath;;
  }
  return _fileId;
}

int File::getFd() {
  return _fd;
}


vector<FileInfo> File::getAllFiles(FileType type) {
  DIR *dir;
  vector<FileInfo> vecFiles;
  vector<FileInfo> tempFiles;
  char path[2048];
  memset(path, 0, 2048);
  strcat(path, DIR_STORAGE);

  dir = opendir (path);
  struct dirent *ent;

  if (dir == NULL) printf("Could not open directory.\n");
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    if(ent->d_name[0] == '.')
      continue;
    if(ent->d_type == DT_REG) {
      //printf ("File Name: %s\n", ent->d_name);
      continue;
    }
    printf("Dir: %s\n", ent->d_name);
    getUserFiles(ent->d_name, type, tempFiles);
    for(unsigned int i = 0; i < tempFiles.size(); i++)
        vecFiles.push_back(tempFiles.at(i));
  }
  closedir (dir);
  return vecFiles;
}

void File::getUserFiles(char* userId, FileType type, vector<FileInfo>& vecFiles) {
  DIR *dir;
  FileInfo fileInfo;
  char username[128];
  char fileId[128];
  char path[2048];
  memset(username, 0, 128);
  memset(fileId, 0, 128);
  memset(path, 0, 2048);

  if(type == Buffer)
      strcat(path, DIR_BUFFER);
  else
      strcat(path, DIR_STORAGE);

  strcat(path, userId);
  if(type == Receive)
      strcat(path, "/recv");
  else if(type == Send)
      strcat(path, "/sent");

  dir = opendir (path);
  struct dirent *ent;
  printf("Current Path: %s\n", path);
  fflush(stdout);
  if (dir == NULL) {
      perror("Could not open directory.\n");
      return;
  }

  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    if(ent->d_name[0] == '.')
      continue;
    if(ent->d_type == DT_REG) {
      sscanf(ent->d_name, "%[^-]%*c%s", username, fileId);
      strcpy(fileInfo.filePath, path);
      strcat(fileInfo.filePath, "/");
      strcat(fileInfo.filePath, ent->d_name);
      strcpy(fileInfo.fileId, fileId);
      strcpy(fileInfo.userId, userId);
      strcpy(fileInfo.recepientId, username);
      vecFiles.push_back(fileInfo);
    }
  }
  closedir (dir);
}

File::~File() {
  unlock();
}
