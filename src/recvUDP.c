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


#define MON_PORT 	7050
#define BUFFER_SIZE 	1024

int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: ./recvUDP <port_local>\n");
		exit(EXIT_FAILURE);
	}

	/* socket */
	int mon_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mon_socket == -1)
	{
		perror("socket:");
		exit(EXIT_FAILURE);
	}

	/* preparer adresse locale */
	struct sockaddr_in adresse;
	socklen_t adrlen;

	adresse.sin_family = AF_INET;
	adresse.sin_port = htons(atoi(argv[1]));
	adresse.sin_addr.s_addr = htonl(INADDR_ANY);
	
	adrlen = sizeof(struct sockaddr_in);

	/* attacher socket à l'adresse */
	if (bind(mon_socket, (struct sockaddr*) &adresse, adrlen) == -1)
	{
		perror("bind:");
		close(mon_socket);
		exit(EXIT_FAILURE);
	}

	/* recevoir message */
	struct sockaddr_in expediteur;	
	char buffer[BUFFER_SIZE];
	
	while(1)
	{

		if (recvfrom(mon_socket, buffer, BUFFER_SIZE, 0, 
			(struct sockaddr *) &expediteur, &adrlen) == -1)
		{
			perror("recvfrom:");
			close(mon_socket);
			exit(EXIT_FAILURE);
		}

		char adr_exp_str[100];
		inet_ntop(AF_INET, &expediteur.sin_addr, adr_exp_str, adrlen);
		int exp_port = ntohs(expediteur.sin_port);
	

		printf("Nouveau message de %s:%d (%lu caractères):\n%s\n",
			adr_exp_str, exp_port, strlen(buffer), buffer);

		memset(buffer, 0, BUFFER_SIZE);
	}
	close(mon_socket);

	return 0;
}

















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

void displayMsg (char *msg) {
	if (msg != NULL)
		printf("%s\n", msg);
	else 
		fprintf(stderr, "Erreur lors de l'affichage du message ; celui-ci est vide...\n");
}

int main (int argc, char* argv[]) {
	
	if (argc < 2) {
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int my_socketfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	
	if (my_socketfd == -1) {
		perror("socket ");
		exit(EXIT_FAILURE);
	}
	
	// récepteur
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[1]));
	
	if ((bind(my_socketfd, (struct sockaddr*) &addr, sizeof(addr))) == -1) {
		perror("bind ");
		exit(EXIT_FAILURE);
	}
	
	
	printf("En attente de reception d'un message... \n");
	
	char* buffer = malloc(BUFFER_SIZE);
	char adr_exp[16];
	adr_exp[15] = '\0';
	
	unsigned int messlen = sizeof(struct sockaddr_in);
	
	struct sockaddr_in expediteur;
	
	recvfrom(my_socketfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &expediteur, &messlen);

	inet_ntop(AF_INET, &(expediteur.sin_addr), adr_exp, sizeof(expediteur)); 
	
	printf("Message recu de %s:%d !\n", adr_exp, ntohs(expediteur.sin_port));
	
	// Affichage du message 
	displayMsg(buffer);
	
	close(my_socketfd);
	
	exit(EXIT_SUCCESS);
}
>>>>>>> orchestrateur
