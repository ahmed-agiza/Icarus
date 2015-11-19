#include "settings.h"

bool      Settings::_initialized = false;
Settings Settings::_instance;
float     Settings::_rcvMsgTimeOut = 0.0;
float     Settings::_sendMsgTimeOut = 0.0;
float     Settings::_replyTimeOut = 0.0;
uint16_t  Settings::_maxRetry = 0;
uint16_t  Settings::_msgBufferSize = 0;
uint16_t  Settings::_threadPool = 0;

Settings& Settings::getInstance(){
  if(!_initialized){
    _initialize();
    _initialized = true;
  }
  return _instance;
}

void Settings::_initialize(){
  FILE *settingsFile = fopen(FILE_LOCATION, "r");
  if(!settingsFile){
    printf("Couldn't open file.\n");
    return;
  }
  char buffer[50];
  char value[50];
  int i = 0;

  while((i = fscanf(settingsFile, "%[^=]=%s\n", buffer, value)) != EOF){
    if(i < 2){
      printf("Failed to getline. %s=%s\n", buffer, value);
      break;
    }
    if(strcmp(BUFFER_SIZE, buffer) == 0)
      _msgBufferSize = atoi(value);
    else if(strcmp(REPLY_TIMEOUT, buffer) == 0)
      _replyTimeOut = atof(value);
    else if(strcmp(RCV_MSG_TIMEOUT, buffer) == 0)
      _rcvMsgTimeOut = atof(value);
    else if(strcmp(SEND_MSG_TIMEOUT, buffer) == 0)
      _sendMsgTimeOut = atof(value);
    else if(strcmp(MAX_RETRY, buffer) == 0)
      _maxRetry = atoi(value);
    else if(strcmp(THREAD_POOL, buffer) == 0)
      _threadPool = atoi(value);
    else
      printf("Failed to parse. %s\n", buffer);
  }
}

Settings::Settings(){

}

Settings::~Settings(){

}

uint16_t Settings::getMessageBufferSize(){
  return _msgBufferSize;
}

uint16_t Settings::getSendMaxRetry(){
  return _maxRetry;
}

uint16_t Settings::getThreadPool(){
  return _threadPool;
}

float Settings::getReplyTimeout(){
  return _replyTimeOut;
}

float Settings::getRcvMsgTimeOut(){
  return _rcvMsgTimeOut;
}

float Settings::getSendMsgTimeOut(){
  return _sendMsgTimeOut;
}
