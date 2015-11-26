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

#define FILE_LOCATION     "../config"
//options
#define BUFFER_SIZE       "MESSAGE_BUFFER_SIZE"
#define REPLY_TIMEOUT     "REPLY_TIMEOUT"
#define RCV_MSG_TIMEOUT   "RCV_MSG_TIMEOUT"
#define SEND_MSG_TIMEOUT  "SEND_MSG_TIMEOUT"
#define MAX_RETRY         "MAX_RETRY"
#define THREAD_POOL       "THREAD_POOL"

class Settings{
private:
Settings();
  static void _initialize();

  static Settings _instance;
  static bool _initialized;
  static uint16_t _msgBufferSize, _maxRetry, _threadPool;
  static float _replyTimeOut, _rcvMsgTimeOut, _sendMsgTimeOut;
public:

  static Settings& getInstance();
  ~Settings();

  uint16_t getMessageBufferSize();
  uint16_t getSendMaxRetry();
  uint16_t getThreadPool();

  float getReplyTimeout();
  float getRcvMsgTimeOut();
  float getSendMsgTimeOut();

};
#endif // SETTINGS_H
