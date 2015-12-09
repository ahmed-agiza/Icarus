#include "client.h"
#include "heartbeat.h"
#include <stdint.h>
#include <string.h>

//#define TEST_ENCDEC
#define TEST_SEEDER
//#define TEST_CLIENT

int main(int argc, char const *argv[]) {

#if defined(TEST_ENCDEC)
  char message[1024];
  char encoded[2048];
  char decoded[2048];
  while(1) {
    memset(message, 0, 1024);
    memset(encoded, 0, 2048);
    memset(decoded, 0, 2048);
    scanf("%s", message);
    size_t encodedSize = 2048, decodedSize = 2048;
    Crypto::base64Encode((unsigned char *)message, strlen(message), encoded, encodedSize);
    printf("Encoded(%d): %s\n", (int)encodedSize, encoded);
    Crypto::base64Decode(encoded, strlen(encoded), (unsigned char *)decoded, &decodedSize);
    printf("Decoded(%d): %s\n", (int)decodedSize, decoded);
  }
  return 0;
#elif defined(TEST_SEEDER)
  if ( argc != 2) {
		printf ("Usage: ./client <server-address>\n");
		exit(1);
	}
  char tempBuff[64];
  char results[MAX_READ_SIZE];
  try {

    HeartBeat *heartBeat = new HeartBeat("ahmed", argv[1], 9999, 0);
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

#elif defined(TEST_CLIENT)
  printf("Client!\n");
  try {
    Client *client = new Client("ahmed", argv[1], 9999, 0);
    int clientRC = client->start();
    client->wait();
    char terminationMessage[LOG_MESSAGE_LENGTH];
    sprintf(terminationMessage, "Client terminated with code %d.", clientRC);
    Logger::info(terminationMessage);
  } catch (NetworkException &e){
    Logger::error(e.what());
  } catch (const char *e) {
    Logger::error(e);
  }

  return 0;
#endif
}
