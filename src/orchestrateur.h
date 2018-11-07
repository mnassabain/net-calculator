#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

using namespace std;

class Node {
    private:
        string command;
        string operations;
        bool state;
        int nbArg;

    public:
        // Constructeurs
        Node();
        Node(struct sockaddr_in ad, int nb, string op);
        Node(struct sockaddr_in addr, string spec);   

        // Change l'état d'un noeud
        void setState(bool newState);      

        // Retourne l'état d'un noeud
        bool getState(void);

        // Retourne l'adresse d'un noeud
        struct sockaddr_in getAddr(void);

        // Retourne l'opération d'un noeud
        string getOp(void);

        // Attribue une commande à un noeud
        void setCommand(string cmd);

        // Retourne la commande atribuée à un noeud
        string getCommand(void);

        // Dernières nouvelles du noeud
        time_t lastHello;

        // @ du noeud
        struct sockaddr_in addr;
};

class Orchestrateur {
    private:
        vector<Node> nodeTab;
        int socketFd;
        struct sockaddr_in addr;
        short port;
        fd_set set;

    public:
        // Affiche le logo
        void afficheLogo(void);

        // Gère la liste des noeuds dans la routine
        void gestionNoeud(struct sockaddr_in addr, string cmd);

        // Trouve un noeud dans la liste de tout les noeuds
        Node* findNode(Node n);

        // Parse l'input utilisateur pour un noeud
        string argToStr(string arg);

        // Utilisation du programme
        void usage(string pname);

        // Ajoute un noeud à la liste des noeuds
        void addNode(Node n);

        // Donne le plus grand fd de l'orchestrateur
        int maxFd(void);

        // Initialise l'@ de l'orchestrateur
        void initAddr(void);

        // Initialise l'ensemble de fds de l'orchestrateur pour select
        void initFdSet(fd_set *set);

        // Nettoye la liste des noeuds pour ceux qui sont trop vieux
        void updateAllNodes(void);

        // Affiche la liste de noeuds de l'orchestrateur
        void displayNodeVec(void);
        
        // Traite une commande parsée de l'utilisateur
        void traiteCmd(string cmd, string calc);

        // Ouvre le terminal de l'orchestrateur
        void openTerm(void);
};
