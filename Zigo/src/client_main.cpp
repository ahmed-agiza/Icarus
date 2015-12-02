#include "client.h"
#include "heartbeat.h"

int main(int argc, char const *argv[]) {
  if ( argc != 2) {
		printf ("Usage: ./client <server-address>\n");
		exit(1);
	}
  char tempBuff[64];
  char results[MAX_READ_SIZE];
  try {
    HeartBeat *heartBeat = new HeartBeat(argv[1], 9999);
    printf("%d\n", heartBeat->start());
    while(1) {
      printf("Input: ");
      memset(tempBuff, 0, 64);
      scanf("%s", tempBuff);
      printf("Read: %s\n", tempBuff);
      if (strcmp(tempBuff, "q") == 0)
        break;
      else if (strcmp(tempBuff, "r") == 0) {
        memset(results, 0, MAX_READ_SIZE);
        printf("Trying to query..\n");
        heartBeat->queryOnline();
        heartBeat->fetchResults(results);
        printf("Query results: %s\n", results);
      }
    }
    printf("Loop exit!\n");
    heartBeat->wait();
  } catch (NetworkException &e){
    Logger::error(e.what());
  } catch (const char *e) {
    Logger::error(e);
  }


  return 0;

  try {
    Client client(argv[1], 9999);
    int clientRC = client.start();
    char terminationMessage[LOG_MESSAGE_LENGTH];
    sprintf(terminationMessage, "Client terminated with code %d.", clientRC);
    Logger::info(terminationMessage);
  } catch (NetworkException &e){
    Logger::error(e.what());
  } catch (const char *e) {
    Logger::error(e);
  }

  return 0;
}
