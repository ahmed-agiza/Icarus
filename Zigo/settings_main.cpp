#include "settings.h"

// MAX_RETRY = 5
// MESSAGE_BUFFER_SIZE = 6500
// REPLY_TIMEOUT = 3.3
// RCV_MSG_TIMEOUT = 3.3
// SEND_MSG_TIMEOUT = 3.3
// THREAD_POOL = 50


int main(){
  int maxRetry = Settings::getInstance().getSendMaxRetry();
  int msgBuffer = Settings::getInstance().getMessageBufferSize();
  int threadPool = Settings::getInstance().getThreadPool();
  float replyTimeout = Settings::getInstance().getReplyTimeout();
  float rcvTimeout = Settings::getInstance().getRcvMsgTimeOut();
  float sendTimeout = Settings::getInstance().getSendMsgTimeOut();

  printf("Value max retry. %i\n", maxRetry);
  printf("Value msg buffer size. %i\n", msgBuffer);
  printf("Value reply timeout. %f\n", replyTimeout);
  printf("Value rcv timeout. %f\n", rcvTimeout);
  printf("Value send timeout. %f\n", sendTimeout);
  printf("Value thread pool. %i\n", threadPool);
  return 0;
}
