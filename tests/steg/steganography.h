#ifndef SETGANOGRAPHY_H
#define SETGANOGRAPHY_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>

#define STEG_FAILED -2

class Steganography {
  static int _embedFile(const char *src, const char *cover, const char *dest, const char *key, bool createNewFile = true);
  static int _extractFile(const char *src, const char *dest, const char *key);
  class File {
    int _fd;
    char _fileId[1024];
    bool _isLocal;
    off_t _offset;
    bool _isOpen;
    bool _isEOF;
    char _filePath[PATH_MAX];
    File() {

    }
  public:
    static File *open(const char *pathname, int flags) {
      File *file = new File;
      memset(file->_filePath, 0, PATH_MAX);
      strcpy(file->_filePath, pathname);
      file->_isLocal = true;
      file->_isOpen = true;
      file->_offset = 0;
      file->_fd = ::open(pathname, flags);
      file->_isEOF = false;
      return file;
    }

    static File *open(const char *pathname, int flags, mode_t mode) {
      File *file = new File;
      memset(file->_filePath, 0, PATH_MAX);
      strcpy(file->_filePath, pathname);
      file->_isLocal = true;
      file->_fd = ::open(pathname, flags, mode);
      file->_isOpen = true;
      file->_offset = 0;
      file->_isEOF = false;
      return file;
    }

    static int remove(const char *filePath) {
      return ::remove(filePath);
    }

    ssize_t write(const void *buf, size_t count) {
      return ::write(_fd, buf, count);
    }
    ssize_t read(void *buf, size_t count) {
      return ::read(_fd, buf, count);
    }

    int remove() const {
      const char *filePath = getFilePath();
      close();
      return remove(filePath);
    }

    const char* getFilePath() const {
      return _filePath;
    }

    static void mkdirRecursive(const char *dir) {
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
    static void createDirIfNotExists(const char *dir) {
      struct stat st = {0};

      if (stat(dir, &st) == -1) {
          mkdirRecursive(dir);
      }
    }
    static void joinPaths(char* destination, const char* path1, const char* path2) {
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
    int close() const {
      return ::close(_fd);
    }
  };
public:
  static int encryptImage(const char *parentPath, const char *embeddedFile, const char *coverFile, const char *saveName, const char* key, bool createNewFile = true);
  static int decryptImage(const char *parentPath, const char *coverFile, const char *extractedFile, const char* key);

  static int encryptImage(const char *parentPath, const char *embeddedFile, const char *coverFile, const char *saveName, const char *extraData, const char* key, bool createNewFile = true);
  static int decryptImage(const char *parentPath, const char *coverFile, const char *extractedFile, char *extraData, size_t dataReadSize, const char* key);

  static int updateImageData(const char *parentPath, const char *coverFile, const char *newData, const char* key);
  static int getImageData(const char *parentPath, const char *coverFile, char *extraData, size_t dataReadSize, const char* key);
};

#endif
