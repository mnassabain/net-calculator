#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define DEST_PORT 	7050
#define BUFFER_SIZE 	1024

int main(int argc, char ** argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: ./sendUDP <@ destination> <port destination> <message>\n");
		exit(EXIT_FAILURE);
	}

	int mon_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mon_socket == -1)
	{
		perror("socket:");
		exit(EXIT_FAILURE);
	}

	/* destinataire */
	struct sockaddr_in dest;
	socklen_t adrlen;

	dest.sin_family = AF_INET;	
	adrlen = sizeof(struct sockaddr_in);

	dest.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &dest.sin_addr);	

	char * message = argv[3];
	int message_size = strlen(message);
	if (sendto(mon_socket, message, message_size, 0,
		(struct sockaddr*) &dest, adrlen) == -1)
	{
		perror("sendto:");
		close(mon_socket);
		exit(EXIT_FAILURE);
	}

	close(mon_socket);

	return 0;
}
