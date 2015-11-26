#ifndef SETTINGS_H
#define  SETTINGS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include "network_exceptions.h"
#include "logger.h"


#define CONFIG_PATH "../config"

//Options
#define OPT_RETRY_TIMES "RETRY_TIMES"
#define OPT_CLIENT_REPLY_TIMEOUT "CLIENT_REPLY_TIMEOUT"
#define OPT_LOG_INFO_PATH "LOG_INFO_PATH"
#define OPT_LOG_ERROR_PATH "LOG_ERROR_PATH"
#define OPT_LOG_LVL "LOG_LVL"
#define OPT_SERVER_REPLY_TIMEOUT "SERVER_REPLY_TIMEOUT"
#define OPT_THREAD_POOL_SIZE "THREAD_POOL_SIZE"

#define PATH_BUFFER_SIZE 1024

class Settings{
private:
Settings();
  void _initialize();

  static Settings _instance;
  static bool _initialized;
  uint32_t _retryTimes, _clientReplyTimeout, _serverReplyTimeout, _poolSize, _logLevel;
  char _logInfoPath[PATH_BUFFER_SIZE], _logErrorPath[PATH_BUFFER_SIZE];
public:

  static Settings& getInstance();
  ~Settings();

  uint32_t getRetryTimes() const;
  uint32_t getClientReplyTimeout() const;
  uint32_t getServerReplyTimeout() const;
  uint32_t getPoolSize() const;
  const char *getLogInfoPath() const;
  const char *getLogErrorPath() const;
  uint32_t getLogLevel() const;

};
#endif // SETTINGS_H
