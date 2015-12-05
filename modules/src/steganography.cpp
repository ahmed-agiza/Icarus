#include "steganography.h"

int Steganography::_embedFile(const char *src, const char *cover, const char *dest, const char *key, bool createNewFile) {
  char command[1024];
  char stdoutBuf[1024];

  if (createNewFile)
    sprintf(command, "steghide embed -ef %s -cf %s -p %s -sf %s -f -q 2>&1", src, cover, key, dest);
  else
    sprintf(command, "steghide embed -ef %s -cf %s -p %s -f -q 2>&1", src, cover, key);
  FILE *processFile = popen(command, "r");

  if (!processFile) {
    fprintf(stderr, "Failed to execute steganography command.\n");
    return STEG_FAILED;
  }

  while (fgets(stdoutBuf, sizeof(stdoutBuf)-1, processFile) != NULL) {
    printf("stdout: %s\n", stdoutBuf);
  }

  int returnCode = pclose(processFile);
  return (returnCode != 0);
}

int Steganography::_extractFile(const char *src, const char *dest, const char *key) {
  char command[1024];
  char stdoutBuf[1024];

  sprintf(command, "steghide extract -sf %s -xf %s -p %s -f -q 2>&1", src, dest, key);

  FILE *processFile = popen(command, "r");
  if (!processFile) {
    fprintf(stderr, "Failed to execute steganography command.\n");
    return STEG_FAILED;
  }

  while (fgets(stdoutBuf, sizeof(stdoutBuf)-1, processFile) != NULL) {
    printf("stdout: %s\n", stdoutBuf);
  }

  int returnCode = pclose(processFile);
  return (returnCode != 0);
}

int Steganography::encryptImage(const char *parentPath, const char *embeddedFile, const char *coverFile, const char *saveName, const char* key, bool createNewFile) {
  char encryptedFilePath[PATH_MAX];
  memset(encryptedFilePath, 0, PATH_MAX);
  File::createDirIfNotExists(parentPath);
  File::joinPaths(encryptedFilePath, parentPath, saveName);
  return _embedFile(embeddedFile, coverFile, encryptedFilePath, key, createNewFile);
}


int Steganography::decryptImage(const char *parentPath, const char *coverFile, const char *extractedFile, const char* key) {
  char decryptedFilePath[PATH_MAX];

  memset(decryptedFilePath, 0, PATH_MAX);
  File::createDirIfNotExists(parentPath);
  File::joinPaths(decryptedFilePath, parentPath, extractedFile);
  return _extractFile(coverFile, decryptedFilePath, key);
}

int Steganography::encryptImage(const char *parentPath, const char *embeddedFile, const char *coverFile, const char *saveName, const char *extraData, const char* key, bool createNewFile) {
  char encryptedFilePath[PATH_MAX], encryptedFileDataPath[PATH_MAX], tempFile[PATH_MAX];
  memset(encryptedFilePath, 0, PATH_MAX);
  memset(encryptedFileDataPath, 0, PATH_MAX);
  memset(tempFile, 0, PATH_MAX);
  File::createDirIfNotExists(parentPath);
  File::joinPaths(encryptedFilePath, parentPath, saveName);
  sprintf(encryptedFileDataPath, "%s.dat", encryptedFilePath);
  sprintf(tempFile, "%s.temp", encryptedFilePath);
  File *dataFile = File::open(encryptedFileDataPath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  dataFile->write(extraData, strlen(extraData));

  int dataEmbeddingRc = _embedFile(encryptedFileDataPath, embeddedFile, tempFile, key);
  if (dataEmbeddingRc) {
    return dataEmbeddingRc;
  }
  dataFile->remove();
  delete dataFile;
  int rc = _embedFile(tempFile, coverFile, encryptedFilePath, key, createNewFile);
  File::remove(tempFile);
  return rc;
}

int Steganography::decryptImage(const char *parentPath, const char *coverFile, const char *extractedFile, char *extraData, size_t dataReadSize, const char* key) {
  char decryptedFilePath[PATH_MAX], decryptedFileDataPath[PATH_MAX];
  memset(decryptedFilePath, 0, PATH_MAX);
  memset(decryptedFileDataPath, 0, PATH_MAX);
  File::createDirIfNotExists(parentPath);
  File::joinPaths(decryptedFilePath, parentPath, extractedFile);
  sprintf(decryptedFileDataPath, "%s.dat", decryptedFilePath);
  int imageDecryptRc = _extractFile(coverFile, decryptedFilePath, key);
  if (imageDecryptRc)
    return imageDecryptRc;
  int dataDecryptRc = _extractFile(decryptedFilePath, decryptedFileDataPath, key);
  File *dataFile = File::open(decryptedFileDataPath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  dataFile->read(extraData, dataReadSize);
  dataFile->remove();
  delete dataFile;

  return dataDecryptRc;
}

int Steganography::updateImageData(const char *parentPath, const char *coverFile, const char *newData, const char* key) {
  char decryptedFilePath[PATH_MAX], enryptedFileDataPath[PATH_MAX];;
  memset(decryptedFilePath, 0, PATH_MAX);
  memset(enryptedFileDataPath, 0, PATH_MAX);
  File::joinPaths(decryptedFilePath, parentPath, "temp-steg-file");
  sprintf(enryptedFileDataPath, "%s.dat", decryptedFilePath);

  int imageDecryptRc = decryptImage(parentPath, coverFile, "temp-steg-file", key);
  if (imageDecryptRc)
    return imageDecryptRc;

  int encryptRc = encryptImage(parentPath, decryptedFilePath, coverFile, "", newData, key, false);
  File::remove(decryptedFilePath);
  return encryptRc;
}

int Steganography::getImageData(const char *parentPath, const char *coverFile, char *extraData, size_t dataReadSize, const char* key) {
  char decryptedFilePath[PATH_MAX];
  memset(decryptedFilePath, 0, PATH_MAX);
  File::joinPaths(decryptedFilePath, parentPath, "temp-steg-file");
  int rc = decryptImage(parentPath, coverFile, "temp-steg-file", extraData, dataReadSize, key);
  File::remove(decryptedFilePath);
  return rc;
}
