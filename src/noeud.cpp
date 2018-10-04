#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string>


#define BUFFER_SIZE     128

#define SIGOK           SIGUSR1
#define SIGNOTOK        SIGUSR2

#define STATUS_OK       1
#define STATUS_NOTOK    0

#define ORCHESTRATEUR_PORT  8000
#define PORT_NOEUD          8001


volatile sig_atomic_t status = STATUS_OK;


class Noeud
{
    private:
        int mon_socket;
        pid_t pid_fils;

        struct sockaddr_in adr_orchestrateur;

        int status;

        std::string profile;

        void pere();
        void fils();

    public:
        Noeud();
        ~Noeud();

        int fonction(int arg1, int arg2);
};

////////////////////////////////////////////////////////////////////////////////
/*
void sighandlerOK(int sig)
{
    int status = STATUS_OK;
}


void sighandlerNOTOK(int sig)
{
    status = STATUS_NOTOK;
}
*/
////////////////////////////////////////////////////////////////////////////////
Noeud::Noeud()
{
    // profile pour identifier
    profile = "+:2";

    status = STATUS_OK;

    // créer socket
    mon_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mon_socket == -1)
    {
        perror("socket:");
        exit(EXIT_FAILURE);
    }

    // adresse de l'orchestrateur
    adr_orchestrateur.sin_family        = AF_INET;
    adr_orchestrateur.sin_port          = htons(ORCHESTRATEUR_PORT);
    adr_orchestrateur.sin_addr.s_addr   = INADDR_ANY;

    // fork
    switch((pid_fils = fork()))
    {
        case -1:
            perror("fork:");
            exit(EXIT_FAILURE);

        case 0:
            fils();
        
        default:
            pere();
    }
}


Noeud::~Noeud()
{
    if (close(mon_socket) == -1)
    {
        perror("close:");
        exit(EXIT_FAILURE);
    }
}


void Noeud::fils()
{
    int status = STATUS_OK;

    /*
    if (signal(SIGOK, sighandlerOK))
    {
        perror("signal:");
        close(mon_socket);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGNOTOK, sighandlerNOTOK))
    {
        perror("signal:");
        close(mon_socket);
        exit(EXIT_FAILURE);
    }
    */

    while(status == STATUS_OK)
    {
        if (sendto(mon_socket, profile.c_str(), profile.size(), 0, 
            (struct sockaddr*) &adr_orchestrateur,
            sizeof(struct sockaddr_in)) == -1)
        {
            perror("sendto:");
            exit(EXIT_FAILURE);
        }

        sleep(5);
    }

    exit(EXIT_SUCCESS);
}


void Noeud::pere()
{
    // pere doit être en ecoute
    // preparer adresse locale
    struct sockaddr_in adresse;
    socklen_t adrlen;

    adresse.sin_family = AF_INET;
    adresse.sin_port    = htons(PORT_NOEUD) ;
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);

    adrlen = sizeof(struct sockaddr_in);

    /* attacher socket à l'adresse */
    if (bind(mon_socket, (struct sockaddr*) &adresse,
        adrlen) == -1)
    {
        perror("bind:");
        close(mon_socket);
        exit(EXIT_FAILURE);
    }

    /* recevoir message */
    int i = 3;
    while(i--)
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        memset(buffer, 0, BUFFER_SIZE);

        if (recv(mon_socket, buffer, BUFFER_SIZE, 0) == -1)
        {
            perror("recv:");
            exit(EXIT_FAILURE);
            close(mon_socket);
        }

        /* reçu message */
        if (kill(pid_fils, SIGNOTOK) == -1)
        {
            perror("kill:");
            close(mon_socket);
            exit(EXIT_FAILURE);
        }

        /* decoder */
        std::string message(buffer);
        std::cout << message << std::endl;

        char c;
        int arg1, arg2;
        std::stringstream stream(message);
        stream >> c >> c >> arg1 >> c >> arg2;

        int res = fonction(arg1, arg2);
        std::cout << c << arg1 << ", " << arg2 << " = " << res << std::endl;

        /* fini et libèré */
        if (kill(pid_fils, SIGNOTOK) == -1)
        {
            perror("kill:");
            close(mon_socket);
            exit(EXIT_FAILURE);
        }
    }
    

    int fils_status;
    if (wait(&fils_status) == -1)
    {
        perror("fils:");
        exit(EXIT_FAILURE);
    }
}



int Noeud::fonction(int arg1, int arg2)
{
    sleep(10);

    return arg1 + arg2;
}



////////////////////////////////////////////////////////////////////////////////


int main(int argc, char ** argv)
{
    Noeud noeud;

    return 0;
}