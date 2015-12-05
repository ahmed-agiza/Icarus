#include "file.h"

File::File() {

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
  file->_isEOF = false;
  return file;
}

File *File::open(const char *pathname, int flags, mode_t mode) {
  File *file = new File;
  file->_isLocal = true;
  file->_socket = 0;
  file->_fd = ::open(pathname, flags, mode);
  file->_isOpen = true;
  file->_offset = 0;
  file->_isEOF = false;
  return file;
}
File *File::ropen(UDPSocket *socket, char *fileId, const char *userId) {
  File *file = new File;
  file->_socket = socket;
  strcpy(file->_userId, userId);
  Message openMessage(Open, fileId, file->_userId, DEFAULT_MESSAGE_ID);
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

  file->_fd = atoi(replyMessage.getBody());
  file->_isOpen = true;
  file->_isLocal = false;
  file->_isEOF = false;
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

ssize_t File::read(void *buf, size_t count) {
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
    Message readMessage(Read, params, _userId, DEFAULT_MESSAGE_ID);
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


ssize_t File::write(const void *buf, size_t count) {
  if (isLocal()) {
    lseek(_fd, SEEK_SET, _offset);
    ssize_t writtenByes = ::write(_fd, buf, count);
    printf("Wrote %zd", writtenByes);
    _offset += writtenByes;
    return writtenByes;
  } else {
    char params[5860];
    sprintf(params, "%d\n%zd\n%s", _fd, count, (char *)buf);
    Message writeMessage(Write, params, _userId, DEFAULT_MESSAGE_ID);
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

const char *File::getFileId() const {
  return _fileId;
}

void File::setOffset(off_t offset) {
  if (isLocal()) {
    _offset = offset;
  } else {
    char seekOff[32];
    memset(seekOff, 0, 32);
    sprintf(seekOff, "%d\n%ld", _fd, (long)offset);
    Message seekMessage(Lseek, seekOff, _userId, DEFAULT_MESSAGE_ID);
    ssize_t sentBytes = _socket->sendMessage(seekMessage);
    (void) sentBytes;
    printf("Offset: %ld\nSent: %s\n", (long) offset, seekOff);

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
      return ::close(_fd);
    } else
      return -1;
  } else {
    char fd[32];
    memset(fd, 0, 32);
    sprintf(fd, "%d", _fd);
    printf("Closing remote file %s\nuMessage closeMessage(Close, fd)", fd);
    fflush(stdout);
    Message closeMessage(Close, fd, _userId, DEFAULT_MESSAGE_ID);
    uint32_t clientReplyTo = Settings::getInstance().getClientReplyTimeout();
    ssize_t sentBytes = _socket->sendMessage(closeMessage);
    (void) sentBytes;

    Message replyMessage = _socket->recvMessageTimeout(clientReplyTo, 0);
    if (replyMessage.getType() != Reply) {
      char invalidReplyMessage[LOG_MESSAGE_LENGTH];
      sprintf(invalidReplyMessage, "Invalid reply: %s", replyMessage.getBytes());
      Logger::error(invalidReplyMessage);
      throw InvalidReplyException();
    }
    _isOpen = false;
    return atoi(replyMessage.getBody());
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

const char *File::getFilePath() const {
  if (isLocal()) {
    return _filePath;;
  }
  return _fileId;
}

int File::getFd() {
  return _fd;
}

File::~File() {

}
