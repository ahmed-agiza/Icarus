#include "logger.h"

int Logger::info(char const *message) {
  if (Settings::getInstance().getLogLevel() <= Verbose) {
    return _logMessage(LOG_INFO_PATH, "Infromation", message, stdout);
  }
  return INVALID_LOG_LEVEL;
}
int Logger::warn(char const *message) {
  if (Settings::getInstance().getLogLevel() <= Warning) {
    return _logMessage(LOG_ERROR_PATH, "Warning", message, stderr);
  }
  return INVALID_LOG_LEVEL;
}
int Logger::error(char const *message) {
  if (Settings::getInstance().getLogLevel() <= Error) {
    return _logMessage(LOG_ERROR_PATH, "Error", message, stderr);
  }
  return INVALID_LOG_LEVEL;
}

tm Logger::_getCurrentTime() {
  time_t t = time(NULL);
  tm currentTime = *localtime(&t);
  return currentTime;
}

int Logger::_logMessage(char const *filePath, char const *prefix, char const *message, FILE *outFile) {
  FILE *logFile = fopen(filePath, "a+");

  if (logFile) {
    tm currentTime = _getCurrentTime();

    fprintf(logFile, "%s(%d-%d-%d %d:%d:%d): %s\n", prefix, currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);

    fclose(logFile);

    if (LOG_STD) {
      fprintf(outFile, "%s(%d-%d-%d %d:%d:%d): %s\n", prefix, currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);
    }

    return WRITE_SUCCESS;
  }

  fprintf(stderr, "Could not open the log file for writing.\n");

  return WRITE_FAILED;
}
