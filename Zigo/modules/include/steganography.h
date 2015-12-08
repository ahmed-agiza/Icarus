#ifndef SETGANOGRAPHY_H
#define SETGANOGRAPHY_H

#include <string.h>
#include <error.h>
#include <linux/limits.h>
#include "file.h"

#define STEG_FAILED -2

class Steganography {
  static int _embedFile(const char *src, const char *cover, const char *dest, const char *key, bool createNewFile = true);
  static int _extractFile(const char *src, const char *dest, const char *key);

public:
  static int encryptImage(const char *parentPath, const char *embeddedFile, const char *coverFile, const char *saveName, const char* key, bool createNewFile = true);
  static int decryptImage(const char *parentPath, const char *coverFile, const char *extractedFile, const char* key);

  static int encryptImage(const char *parentPath, const char *embeddedFile, const char *coverFile, const char *saveName, const char *extraData, const char* key, bool createNewFile = true);
  static int decryptImage(const char *parentPath, const char *coverFile, const char *extractedFile, char *extraData, size_t dataReadSize, const char* key);

  static int updateImageData(const char *parentPath, const char *coverFile, const char *newData, const char* key);
  static int getImageData(const char *parentPath, const char *coverFile, char *extraData, size_t dataReadSize, const char* key);
};

#endif
