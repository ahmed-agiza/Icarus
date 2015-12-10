#include "client.h"

int main(int argc, char *argv[]){
  if ( argc != 2) {
		printf ("Usage: ./client <server-address>\n");
		exit(1);
	}

  try {
    Client client(argv[1], 9999);
    printf("Client terminated with code %d.\n", client.start());
  } catch (const char *e) {
    fprintf(stderr, "%s\n", e);
  }

  return 0;
}
