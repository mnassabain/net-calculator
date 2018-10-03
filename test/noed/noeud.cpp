#include "noeud.h"

#include <iostream>
#include <string>

#define BUFFER_SIZE     64

Noeud::Noeud()
{
    // profile pour identifier
    profile = "+:2";

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
    adresse.sin_port    = htons(MON_PORT);
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
        if (recv(mon_socket, buffer, BUFFER_SIZE, 0) == -1)
        {
            perror("recv:");
            exit(EXIT_FAILURE);
            close(mon_socket);
        }

    std::string message(buffer);

    std::cout << message << std::endl;
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