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
#define OPT_RETRY_TIMES                   "RETRY_TIMES"
#define OPT_CLIENT_REPLY_TIMEOUT          "CLIENT_REPLY_TIMEOUT"
#define OPT_LOG_INFO_PATH                 "LOG_INFO_PATH"
#define OPT_LOG_ERROR_PATH                "LOG_ERROR_PATH"
#define OPT_LOG_LVL                       "LOG_LVL"
#define OPT_SERVER_REPLY_TIMEOUT          "SERVER_REPLY_TIMEOUT"
#define OPT_THREAD_POOL_SIZE              "THREAD_POOL_SIZE"

#define OPT_CLIENT_MANAGER_CACHE          "CLIENT_CACHE"
#define OPT_LOG_MESSAGE_LENGTH            "LOG_MESSAGE_LENGTH"
#define OPT_SEEDER_ID                     "SEEDER_ID"
#define OPT_DEFAULT_MESSAGE_ID            "DEFAULT_MESSAGE_ID"
#define OPT_RECENT_PEERS                  "RECENT_PEERS"
#define OPT_AUTO_CREATE                   "AUTO_CREAT_THREADS"

#define OPT_DIR_STORAGE                   "STORAGE_DIR"
#define OPT_DIR_BUFFER                    "BUFFER_DIR"

#define PATH_BUFFER_SIZE 1024

class Settings{
private:
    Settings();
    void _initialize();

    static Settings   _instance;
    static bool       _initialized;
    uint32_t          _retryTimes, _clientReplyTimeout, _serverReplyTimeout, _poolSize, _logLevel,
    _clientCache, _logLen, _recentPeers, _autoCreate;

    char _logInfoPath[PATH_BUFFER_SIZE], _logErrorPath[PATH_BUFFER_SIZE], _seederID[128], _defMsgId[128], _dirStorage[128], _dirBuffer[PATH_BUFFER_SIZE];
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

    uint32_t getClientCache() const;
    uint32_t getLogLen() const;
    uint32_t getRecentCount() const;
    uint32_t getAutoCreate() const;

    const char* getSeederID() const;
    const char* getDefMsgId() const;
    const char* getDirStorage() const;
    const char* getDirBuffer() const;

};
#endif // SETTINGS_H
