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


#define STDIN               0

/* Variables globales qui indiquent si les options -v ou -6 sont activées*/
bool FLAG_V = false;
bool FLAG_6 = false;


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

        /* Socket du noeud */
        int mon_socket;

        /* Structure contenant l'adresse du noeud ainsi que sa longueur */
        struct sockaddr_storage adresse;
        socklen_t adrlen;        

        /* L'adresse de l'orchestrateur */
        struct sockaddr_storage adr_orchestrateur;
        socklen_t adrlen_orchestrateur;

        /* Fonction qui envoie un message vers l'orchestrateur */
        void envoyer_message(std::string& message);

        /* Fonction qui reçoit le message */
        void recevoir_message(int * arg1, int * arg2);

        /* Fonctions auxiliaires */
        void print_time();

        /* Fonctions que vont executer le processus père et fils respectivement 
        */
        void pere();
        void fils();

        /* Identifiant du processus fils */
        pid_t pid_fils;

        /* Dernier message reçu de l'orchestrateur */
        std::string message;

        /* Resultat */
        int resultat;

        static std::string int_to_string(int val);


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
    if (FLAG_V)
    {
        print_time();
        std::cout << "Demarer noeud" << std::endl;
    } 
    profile = "+:2";

    if (FLAG_V)
    {
        print_time();
        std::cout << "Setup adresse du noeud " << (FLAG_6 ? "IPv6" : "IPv4") << std::endl;
    }

    /* Création adresse */
    if (FLAG_6)
    {
        struct sockaddr_in6 * adrv6 = (struct sockaddr_in6*) &adresse;
        adrv6->sin6_family = AF_INET6;
        adrv6->sin6_port = htons(PORT_NOEUD);
        adrv6->sin6_addr = in6addr_any;
    }
    else
    {
        struct sockaddr_in * adrv4 = (struct sockaddr_in*) &adresse;
        adrv4->sin_family = AF_INET;
        adrv4->sin_port = htons(PORT_NOEUD) ;
        adrv4->sin_addr.s_addr = htonl(INADDR_ANY);
    }

    adrlen = sizeof(adresse);
}


/**
 * Fonction: fonction
 * 
 * L'opération du noeud de calcul.
 * 
 */
int Noeud::fonction(int arg1, int arg2)
{
    if (FLAG_V)
    {
        print_time();
        std::cout << "Demarer calcul" << std::endl;
    } 
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
    if (FLAG_V)
    {
        print_time();
        std::cout << "Créer socket " << (FLAG_6 ? "IPv6" : "IPv4") << std::endl;
    } 
    
    if (FLAG_6)
    {
        mon_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        mon_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

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
    if (FLAG_V)
    {
        print_time();
        std::cout << "Setup adresse orchestrateur" << std::endl;
    }

    if (FLAG_6)
    {
        struct sockaddr_in6 * adrv6 = (struct sockaddr_in6*) &adr_orchestrateur;
        adrv6->sin6_family = AF_INET6;
        adrv6->sin6_port = htons(ORCHESTRATEUR_PORT);
        adrv6->sin6_addr = in6addr_any;
    }
    else
    {
        struct sockaddr_in * adrv4 = (struct sockaddr_in*) &adr_orchestrateur;
        adrv4->sin_family = AF_INET;
        adrv4->sin_port = htons(ORCHESTRATEUR_PORT);
        adrv4->sin_addr.s_addr = htonl(INADDR_ANY);
    }

    adrlen_orchestrateur = sizeof(adr_orchestrateur);
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
    if (FLAG_V)
    {
        print_time();
        std::cout << "Créer un fils" << std::endl;
    } 
    switch(pid_fils = fork())
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
            perror("sendto:::");
            exit(EXIT_FAILURE);
        }
}


/**
 * Fonction: recevoir_message
 * 
 */
void Noeud::recevoir_message(int * arg1, int * arg2)
{
    char buffer[BUFFER_SIZE];
    if (recv(mon_socket, buffer, BUFFER_SIZE, 0) == -1)
    {
        perror("recv");
        close(mon_socket);
        exit(EXIT_FAILURE);
    }

    if (FLAG_V)
    {
        print_time();
        std::cout << "Message reçu de l'orchestrateur" << std::endl;
    }

    message = std::string(buffer);
    decoder_commande(message, arg1, arg2);
}


/**
 * Fonction: print_time
 * 
 * Affiche le temps courant sous forme [h:m:s]
 * 
 */
void Noeud::print_time()
{
    time_t temps;
    struct tm * timeinfo;
    char buffer[80];

    time(&temps);
    timeinfo = localtime(&temps);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    std::string str(buffer);
    memset(buffer, '0', sizeof(buffer));
    std::cout << "[" << str << "] ";
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
        if (FLAG_V)
        {
            print_time();
            std::cout << "Ping à l'orchestrateur" << std::endl;
        }
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
    if (FLAG_V)
    {
        print_time();
        std::cout << "Bind socket" << std::endl;
    }
    if (bind(mon_socket, (struct sockaddr*) &adresse, adrlen) == -1)
    {
        perror("bind");
        close(mon_socket);
        exit(EXIT_FAILURE);
    }

    /* recevoir message */
    int i = 3;
    while(i--)
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        fd_set readfds;
        fd_set readfds2;
        int nfds;

        FD_ZERO(&readfds);
        FD_SET(mon_socket, &readfds);
        FD_SET(STDIN, &readfds);

        nfds = mon_socket + 1;
        
        int nr;
        while(1)
        {
            readfds2 = readfds;
            nr = select(nfds, &readfds2, 0, 0, NULL);
            if (nr == -1)
            {
                perror("select");
                exit(EXIT_FAILURE);
            }
            else
            {
                if (FD_ISSET(mon_socket, &readfds2))
                {
                    int arg1, arg2;
                    recevoir_message(&arg1, &arg2);

                    int res = fonction(arg1, arg2);

                    std::string resultat_string = int_to_string(res);

                    if (FLAG_V)
                    {
                        print_time();
                        std::cout << "Envoyer resultat à l'orchestrateur" << std::endl;
                    }

                    envoyer_message(resultat_string);
                }

                if (FD_ISSET(STDIN, &readfds2))
                {
                    if (FLAG_V)
                    {   
                        print_time();
                        std::cout << "Quitter programme" << std::endl;
                    }
                    while((getchar() != '\n'));
                    kill(pid_fils, SIGKILL);
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
}


std::string Noeud::int_to_string(int val)
{
    std::stringstream transformresult;
    transformresult << val;

    return transformresult.str();
}

////////////////////////////////////////////////////////////////////////////////
//      MAIN
////////////////////////////////////////////////////////////////////////////////


int main(int argc, char ** argv)
{
    /* Gestion d'entrée & options */
    if (argc < 1 || argc > 3)
    {
        std::cerr << "Usage: noeud [-v] [-6]" << std::endl;
        exit(EXIT_FAILURE);
    }

    int option;
    //int optind;
    while ((option = getopt(argc, argv, "v6")) != -1)
    {
        switch(option)
        {
            case 'v':
                FLAG_V = true;
                break;

            case '6':
                FLAG_6 = true;
                break;

            default:
                std::cerr << "Usage: noeud [-v] [-6]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    if (optind != argc)
    {
        std::cerr << "Usage: noeud [-v] [-6]" << std::endl;
        exit(EXIT_FAILURE);
    }


    Noeud noeud;
    noeud.creer_socket();
    noeud.trouver_orchestrateur();

    noeud.lancer_noeud();

    return 0;
}
