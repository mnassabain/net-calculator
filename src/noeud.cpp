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

////////////////////////////////////////////////////////////////////////////////
//      STRUCTURES & MACROS
////////////////////////////////////////////////////////////////////////////////

/* La taille du buffer lors la lecture du message de l'orchestrateur */
#define BUFFER_SIZE         128

/* Le port du noeud et de l'orchestrateur respectivement */
#define PORT_NOEUD          8001
#define ORCHESTRATEUR_PORT  8000

/* Le temps que le noeud attend pour envoyer son profil à l'orchestrateur */
#define TEMPS_SIGNAL        5

/* Le temps que le noeud doit attendre pour envoyer le calcul */
#define TEMPS_CALCUL        5


/* Structure du noeud */
class Noeud
{
    private:
        /* informations sur le noeud */
        
        /* profile du noeud sous la forme operation:nombre_arguments */
        std::string profile;

        /* La fonction du noeuds */
        int fonction(int arg1, int arg2);

        /* La fonction qui decode la commande et place les arguments */
        void decoder_commande(std::string& message, int * arg1, int * arg2);

        /* Structure contenant l'adresse du noeud ainsi que sa longueur */
        struct sockaddr_in adresse;
        socklen_t adrlen;           

        /* Socket du noeud */
        int mon_socket;

        /* L'adresse de l'orchestrateur */
        struct sockaddr_in adr_orchestrateur;
        socklen_t adrlen_orchestrateur;

        /* Fonction qui envoie un message vers l'orchestrateur */
        void envoyer_message(std::string& message);

        /* Fonctions que vont executer le processus père et fils respectivement 
        */
        void pere();
        void fils();


    public:
        /* Constructeur et deconstructeur */
        Noeud();
        ~Noeud();

        /* Fonction qui crée le socket */
        void creer_socket();

        /* Crée et remplit la structure contenant l'adresse de l'orchestrateur
        */
        void trouver_orchestrateur();

        /* Lancer le noeud */
        void lancer_noeud();
};


////////////////////////////////////////////////////////////////////////////////
//      DEFINTION DES FONCTIONS
////////////////////////////////////////////////////////////////////////////////

/**
 * Fonction: Constructeur
 * 
 * Remplit la structure contenant l'adresse et le port du noeud, ainsi que son
 * profile.
 * 
*/
Noeud::Noeud()
{
    profile = "+:2";

    adresse.sin_family = AF_INET;
    adresse.sin_port    = htons(PORT_NOEUD) ;
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);

    adrlen = sizeof(struct sockaddr_in);
}


/**
 * Fonction: fonction
 * 
 * L'opération du noeud de calcul.
 * 
 */
int Noeud::fonction(int arg1, int arg2)
{
    sleep(TEMPS_CALCUL);

    return arg1 + arg2;
}


/* La fonction qui decode la commande et place les arguments */
void Noeud::decoder_commande(std::string& message, int * arg1, int * arg2)
{
    std::cout << message << std::endl;

    char c;
    std::stringstream stream(message);
    stream >> c >> c >> *arg1 >> c >> *arg2;
}

/**
 * Fonction: créer_socket
 * 
 * Crée le socket en mode datagramme (UDP) et le stocke dans le noeud. S'il y a 
 * une erreur on ferme le programme
 * 
 */
void Noeud::creer_socket()
{
    mon_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mon_socket == -1)
    {
        perror("socket:");
        exit(EXIT_FAILURE);
    }
}


/**
 * Fonction: trouver_orchestrateur
 * 
 * Remplit la structure contenant l'adresse de l'orchestrateur
 * 
 */
void Noeud::trouver_orchestrateur() 
{
    // adresse de l'orchestrateur
    adr_orchestrateur.sin_family        = AF_INET;
    adr_orchestrateur.sin_port          = htons(ORCHESTRATEUR_PORT);
    adr_orchestrateur.sin_addr.s_addr   = INADDR_ANY;

    adrlen_orchestrateur = sizeof(struct sockaddr_in);
}


/**
 * Fonction: lancer_noeud
 * 
 * Fait un fork pour créer un processus fils qui va envoyer un signal chaque
 * TEMPS_SIGNAL secondes. Le père reste en écoute pour une instruction.
 * 
 */
void Noeud::lancer_noeud()
{
    switch(fork())
    {
        case -1:
            perror("fork:");
            close(mon_socket);
            exit(EXIT_FAILURE);

        case 0:
            fils();
        
        default:
            pere();
    }
}


/**
 * Fonction: Destructeur
 * 
 * Ferme le socket
 *
 */
Noeud::~Noeud()
{
    if (close(mon_socket) == -1)
    {
        perror("close:");
        exit(EXIT_FAILURE);
    }
}


/**
 * Fonction: envoyer_message
 * 
 * Arguments:
 *  message - Le message à envoyer
 * 
 * Envoie un message à l'orchestrateur
 * 
 */
void Noeud::envoyer_message(std::string& message) 
{
    if (sendto(mon_socket, message.c_str(), message.size(), 0, 
        (struct sockaddr*) &adr_orchestrateur,
        adrlen_orchestrateur) == -1)
        {
            perror("sendto:");
            exit(EXIT_FAILURE);
        }
}


/**
 * Fonction: fils
 * 
 * La fonction que le fils va executer. Il va envoyer un message à 
 * l'orchestrateur contenant son profil chaque TEMPS_SIGNAL secondes.
 * 
 */
void Noeud::fils()
{
    while(1)
    {
        envoyer_message(profile);

        sleep(TEMPS_SIGNAL);
    }

    exit(EXIT_SUCCESS);
}


/**
 * Fonction: pere
 * 
 * La fonction que le père va executer. Va lier son adresse au socket. Se met en
 * écoute bloquante (attente passive) tant qu'il reçoit un message de 
 * l'orchestrateur.
 * Quand il reçoit une commande il calcule le résultat et il l'envoie à 
 * l'orchestrateur.
 * 
 */
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
            close(mon_socket);
            exit(EXIT_FAILURE);
        }

        std::string message(buffer);
        int arg1, arg2;
        decoder_commande(message, &arg1, &arg2);

        int res = fonction(arg1, arg2);

        /* int -> string */
        std::stringstream transformresult;
        transformresult << res;

        std::string result = transformresult.str();
        envoyer_message(result);
    }
    

    int fils_status;
    if (wait(&fils_status) == -1)
    {
        perror("fils:");
        exit(EXIT_FAILURE);
    }
}

////////////////////////////////////////////////////////////////////////////////
//      MAIN
////////////////////////////////////////////////////////////////////////////////


int main(int argc, char ** argv)
{
    Noeud noeud;
    noeud.creer_socket();
    noeud.trouver_orchestrateur();

    noeud.lancer_noeud();

    return 0;
}