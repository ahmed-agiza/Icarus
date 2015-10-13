#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void terminate_with_error (int sock)
{
	close (sock);
	perror("Error Binding Socket:");
	exit(1);

}

int main (int argc,char ** argv)
{
	if ( argc != 3)
	{
		printf ("Usage: ./client <server-address>\n");
		exit(1);
	}
	int sock;
	struct sockaddr_in serverAddr;
	socklen_t sin_size = sizeof(struct sockaddr_in);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Error Creating Socket");
		exit(1);
	}
	memset((char *) &serverAddr, 0,sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9999);
        struct hostent *server = gethostbyname(argv[1]);
	if ( server == NULL ) terminate_with_error(sock);
	memcpy((char *)&serverAddr.sin_addr.s_addr,(char *)server->h_addr, server->h_length);
        memset(&(serverAddr.sin_zero), 0, 8);
	sendto(sock,argv[2], strlen(argv[2]),0,(sockaddr *)&serverAddr,sin_size);
	close(sock);
}
