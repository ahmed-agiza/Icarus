#include "settings.h"

bool Settings::_initialized = false;
Settings Settings::_instance;

Settings& Settings::getInstance(){
  if(!_initialized){
    _instance._initialize();
    _initialized = true;
  }
  return _instance;
}

void Settings::_initialize(){
  FILE *settingsFile = fopen(CONFIG_PATH, "r");
  if(!settingsFile){
    Logger::error("Failed to open settings file.");
    throw SettingsLoadException();
    return;
  }
  char buffer[50];
  char value[50];
  int i = 0;

  while((i = fscanf(settingsFile, "%[^=]=%s\n", buffer, value)) != EOF){
    if(i < 2){
      char getlineErrorMessage[LOG_MESSAGE_LENGTH];
      sprintf(getlineErrorMessage, "Failed to getline. %s=%s\n", buffer, value);
      Logger::error(getlineErrorMessage);
      throw InvalidOptionException();
      break;
    }

    if(strcmp(OPT_RETRY_TIMES, buffer) == 0)
      _retryTimes = atoi(value);
    else if(strcmp(OPT_CLIENT_REPLY_TIMEOUT, buffer) == 0)
      _clientReplyTimeout = atof(value);
    else if(strcmp(OPT_LOG_INFO_PATH, buffer) == 0)
      strcpy(_logInfoPath, value);
    else if(strcmp(OPT_SERVER_REPLY_TIMEOUT, buffer) == 0)
      _serverReplyTimeout = atof(value);
    else if(strcmp(OPT_LOG_ERROR_PATH, buffer) == 0)
      strcpy(_logErrorPath, value);
    else if(strcmp(OPT_LOG_LVL, buffer) == 0)
      _logLevel = (uint32_t)strtoull(value, NULL, 0);
    else if(strcmp(OPT_THREAD_POOL_SIZE, buffer) == 0)
      _poolSize = atoi(value);
    else if(strcmp(OPT_CLIENT_MANAGER_CACHE, buffer) == 0)
      _clientCache = atoi(value);
    else if(strcmp(OPT_RECENT_PEERS, buffer) == 0)
      _recentPeers = atoi(value);
    else if(strcmp(OPT_LOG_MESSAGE_LENGTH, buffer) == 0)
      _logLen = atoi(value);
    else if(strcmp(OPT_AUTO_CREATE, buffer) == 0)
      _autoCreate = atoi(value);
    else if(strcmp(OPT_SEEDER_ID, buffer) == 0)
      strcpy(_seederID, value);
    else if(strcmp(OPT_DEFAULT_MESSAGE_ID, buffer) == 0)
      strcpy(_defMsgId, value);
    else if(strcmp(OPT_DIR_STORAGE, buffer) == 0)
      strcpy(_dirStorage, value);
    else if(strcmp(OPT_DIR_BUFFER, buffer) == 0)
      strcpy(_dirBuffer, value);
    else{
      char invaliOptionMessage[LOG_MESSAGE_LENGTH];
      sprintf(invaliOptionMessage, "Failed to parse %s %s\n", buffer, value);
      Logger::error(invaliOptionMessage);
      throw InvalidOptionException();
    }
  }
}

Settings::Settings(){

}

Settings::~Settings(){

}

uint32_t Settings::getRetryTimes() const {
  return _retryTimes;
}

uint32_t Settings::getClientReplyTimeout() const {
  return _clientReplyTimeout;
}

uint32_t Settings::getServerReplyTimeout() const {
  return _serverReplyTimeout;
}

uint32_t Settings::getPoolSize() const {
  return _poolSize;
}

const char *Settings::getLogInfoPath() const {
  return _logInfoPath;
}

const char *Settings::getLogErrorPath() const {
  return _logErrorPath;
}

uint32_t Settings::getLogLevel() const {
  return _logLevel;
}

uint32_t Settings::getClientCache() const {
  return _clientCache;
}

uint32_t Settings::getLogLen() const {
  return _logLen;
}

uint32_t Settings::getRecentCount() const {
  return _recentPeers;
}
uint32_t Settings::getAutoCreate() const {
  return _autoCreate;
}

const char* Settings::getSeederID() const {
  return _seederID;
}
const char* Settings::getDefMsgId() const {
  return _defMsgId;
}

const char* Settings::getDirStorage() const {
  return _dirStorage;
}
const char* Settings::getDirBuffer() const {
  return _dirBuffer;
}
