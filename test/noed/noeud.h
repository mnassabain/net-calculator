#ifndef __NOEUD_H__
#define __NOEUD_H__


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

#define SIGOK           SIGUSER1
#define SIGNOTOK        SIGUSER2

#define STATUS_OK       1
#define STATUS_NOTOK    0

#define ORCHESTRATEUR_PORT  8000
#define MON_PORT            8001


class Noeud
{
    private:
        int mon_socket;
        pid_t pid_fils;

        struct sockaddr_in adr_orchestrateur;

        std::string profile;

        void pere();
        void fils();

    public:
        Noeud();
        ~Noeud();

        int fonction(int arg1, int arg2);
};





#endif