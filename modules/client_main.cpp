#include "client.h"

int main(int argc, char *argv[]){
  //uint16_t port = argv[2];
  if ( argc != 2) {
		printf ("Usage: ./client <server-address>\n");
		exit(1);
	}
  Client client(argv[1], 9999);
  printf("Client terminated with code %d.\n", client.start());

  return 0;
}
