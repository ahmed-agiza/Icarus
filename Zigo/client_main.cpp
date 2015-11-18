#include "client.h"

int main(int argc, char const *argv[]) {
  if ( argc != 2) {
		printf ("Usage: ./client <server-address>\n");
		exit(1);
	}

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
