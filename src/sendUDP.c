<<<<<<< HEAD
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
=======
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512

struct sockaddr_in createAddr(char *addresse, short port) {
	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(addresse);
	addr.sin_port = htons(port);
	
	return addr;
}

int createSocket(void) {
	int socketFd = 0;
	
	if ((socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket creation ");
		exit(EXIT_FAILURE);
	}
	
	return socketFd;
}

char* createBuffer(const char *msg) {
	int size = strlen(msg);
	
	char *buffer = malloc(size+1);
	
	if (buffer == NULL) {
		fprintf(stderr, "Error w/ malloc\n");
		exit(EXIT_FAILURE);
	}
	
	*(buffer + size) = '\0';
	
	int i = 0;
	for (i=0 ; i<size ; i++) {
		buffer[i] = msg[i];
	}
		
	return buffer;
}

int main (int argc, char* argv[]) {
	
	if (argc != 5) {
		fprintf(stderr, "USAGE: %s <addr> <port> <message> <myport>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// Creation d'un socket
	int socketFd = createSocket();
	
	// Creation d'une addr
	struct sockaddr_in addr = createAddr(argv[1], (short) atoi(argv[2]));

	struct sockaddr_in addr_of_me = createAddr("127.0.0.1", atoi(argv[4]));

	if ((bind(socketFd, (struct sockaddr*) &addr_of_me, sizeof(addr_of_me))) == -1) {
		perror("bind ");
		exit(EXIT_FAILURE);
	}
	
	// Allocation du buffer
	char *buff = createBuffer(argv[3]);
	
	//sleep(10);

	//Envoi du message
	if ((sendto(socketFd, buff, strlen(buff)+1, 0, (struct sockaddr*) &addr, sizeof(addr))) < 0) {
		perror("send message ");
		exit(EXIT_FAILURE);
	}

	free(buff);
	printf("Le message a été envoyé avec succès !\n");
	
	close(socketFd);
	
	exit(EXIT_SUCCESS);
>>>>>>> orchestrateur
}
