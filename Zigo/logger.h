#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define LOG_ERROR_PATH "error.log"
#define LOG_INFO_PATH "info.log"

#define LOG_MESSAGE_LENGTH 100

#define WRITE_FAILED -1
#define WRITE_SUCCESS 0
#define INVALID_LOG_LEVEL -2

#define LOG_STD 1


enum LogLevel {
  Verbose = 0,
  Warning = 1,
  Error = 2
};

#define LOG_LVL Verbose

class Logger {
  static tm _getCurrentTime();
  static int _logMessage(char const *filePath, char const *prefix, char const *message, FILE *outFile = stdout);
public:
  static int info(char const *message);
  static int warn(char const *message);
  static int error(char const *message);
};

#endif
