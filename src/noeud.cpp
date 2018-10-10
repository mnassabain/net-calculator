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

#define ORCHESTRATEUR_PORT  8000
#define PORT_NOEUD          8001



class Noeud
{
    private:
        std::string profile;
        struct sockaddr_in adresse; // mon adresse
        socklen_t adrlen;           // longueur de l'adresse

        int mon_socket;
        pid_t pid_fils;

        struct sockaddr_in adr_orchestrateur;

        int fonction(int arg1, int arg2);
        void envoyer_message(std::string& message);

        void pere();
        void fils();


    public:
        Noeud();
        ~Noeud();

        void creer_socket();
        void trouver_orchestrateur();
        void lancer_noeud();
};


Noeud::Noeud()
{
    // profile de la forme "operation:nb_arguments"
    profile = "+:2";

    adresse.sin_family = AF_INET;
    adresse.sin_port    = htons(PORT_NOEUD) ;
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);

    adrlen = sizeof(struct sockaddr_in);
}


void Noeud::creer_socket()
{
    mon_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mon_socket == -1)
    {
        perror("socket:");
        exit(EXIT_FAILURE);
    }
}


void Noeud::trouver_orchestrateur() 
{
    // adresse de l'orchestrateur
    adr_orchestrateur.sin_family        = AF_INET;
    adr_orchestrateur.sin_port          = htons(ORCHESTRATEUR_PORT);
    adr_orchestrateur.sin_addr.s_addr   = INADDR_ANY;
}


void Noeud::lancer_noeud()
{
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


void Noeud::envoyer_message(std::string& message) 
{
    if (sendto(mon_socket, message.c_str(), message.size(), 0, 
        (struct sockaddr*) &adr_orchestrateur,
        sizeof(struct sockaddr_in)) == -1)
        {
            perror("sendto:");
            exit(EXIT_FAILURE);
        }
}


void Noeud::fils()
{
    while(1)
    {
        envoyer_message(profile);

        sleep(5);
    }

    exit(EXIT_SUCCESS);
}


void Noeud::pere()
{
    /* attacher socket à l'adresse */
    if (bind(mon_socket, (struct sockaddr*) &adresse, adrlen) == -1)
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

        if (recv(mon_socket, buffer, BUFFER_SIZE, 0) == -1)
        {
            perror("recv:");
            exit(EXIT_FAILURE);
            close(mon_socket);
        }

        /* decoder */
        std::string message(buffer);
        std::cout << message << std::endl;

        char c;
        int arg1, arg2;
        std::stringstream stream(message);
        stream >> c >> c >> arg1 >> c >> arg2;

        int res = fonction(arg1, arg2);

        std::stringstream resultat_message;
        resultat_message << message << " = " << res;
        
        std::string resultat_message_str = resultat_message.str();
        envoyer_message(resultat_message_str);
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
    sleep(5);

    return arg1 + arg2;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char ** argv)
{
    Noeud noeud;
    noeud.creer_socket();
    noeud.trouver_orchestrateur();

    noeud.lancer_noeud();

    return 0;
}