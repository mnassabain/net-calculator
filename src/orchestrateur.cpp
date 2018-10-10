#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <string>
#include <iostream>

#define TRUE            1
#define FALSE           0
#define BUFFSIZE        512
#define MAX_CMD_CHAR    256

using namespace std;

void usage (string pname);

void affiche_logo(void) {
    char buff[BUFFSIZE];
    int bufflen = 0;

    printf("Projet de réseaux et protocoles / L3-S5A 2018\n");
    printf("NASSABIN Marco et WENDLING Nicolas - 2018\n");

    int fd = open("logo.txt", O_RDONLY);
    if (fd < 0) {
        perror("open logo ");
        exit(EXIT_FAILURE);
    }

    while ((bufflen = read(fd, buff, BUFFSIZE)) > 0) {
        if (write(1, buff, bufflen) == -1) {
            perror("stdout writing ");
            exit(EXIT_FAILURE);
        }
    }

    if (bufflen == -1) {
        perror("reading the logo file ");
        exit(EXIT_FAILURE);
    }

    if (close(fd) < 0) {
        perror("closing the logo file ");
        exit(EXIT_FAILURE);
    }
}

string lireCmd (void) {
    char res[MAX_CMD_CHAR];
    string str;

    cin.getline(res, MAX_CMD_CHAR);

    str = res;

    return str;
}

// Cette fonction transforme une commande de calcul avec des arguments en string
string argToString (string arg) {
    int size = 0, i = 0;
    string str;
    size = arg.size();
    char c;

    while ((i<size) && ((c = arg.at(i)) != '(')) {
        if (c == '%' || c == '+' || c == '-' || c == '*' || c == '/' || c == '!')
            str.push_back(arg[i]);
            
        i++;
    }

    i++;

    if ((str.empty()) || (i >= size)) {
        cerr << "Syntaxe incorrecte, veuillez réessayer" << endl;
        usage("./Orchestrateur");
        return "";
    }

    else {
      
        // Séparateur
        str.push_back(':');

       unsigned int nbOperande = str.size() - 1;
       unsigned int nbArgs = 0;
    
        while(((i < size) && (c = arg.at(i)) != ')')) {
            if ((c >= '0') && (c <= '9')) {
                str.push_back(c);
                nbArgs++;
            }
            else if (c == ',') {
                str.push_back(':');
            }

            else if (c == ' ') { i++; continue; }

            else {
                cerr << "Mauvais argument" << endl;
                usage("./Orchestrateur");
                return "";
            }

            i++;
        }

        // Si il n'y a aucun argument...
        if (str.size() == nbOperande + 1) {
            cerr << "Argument incorrect" << endl;
            usage("./Orchestrateur");
            return "";
        }

        return str;
    }
}

void searchAddrNode(string calc, struct sockaddr_in *addr) {
    return;
}

void traitement_calcul(string calc) {
    pid_t pid;

    pid = fork();

    switch(pid) {
        case -1:
            perror("cannot fork ");
            exit(EXIT_FAILURE);
            break;

        case 0: // fils
            struct sockaddr_in addrNode;
            // On initialise l'@ à 0 comme ça si on a tjrs cette @ même après 
            // la recherche, celà veut dire qu'aucun noeud n'est disponible
            addrNode.sin_addr.s_addr = (inet_addr("0.0.0.0"));
            // On cherche l'@ du noeud correspond au calcul que l'on veut faire
            searchAddrNode(calc, &addrNode);

            if (addrNode.sin_addr.s_addr == 0) {
                cerr << "Aucun noeud n'est disponible pour votre calcul veuillez réessayer ultérieurement..." << endl;
            }

            exit(EXIT_SUCCESS);
            break;
            
        default: // pere
            int fils_status;
            if (wait(&fils_status) == -1) {
                perror("fils:");
                exit(EXIT_FAILURE);
            }

            break;
    }
}

void openTerm (void) {

    affiche_logo();

    string cmd;
    bool quit = false;
    

    do {

        printf("\norchestrateur> ");
        cmd = lireCmd();

        if ((!cmd.empty()) && (cmd.size() <= MAX_CMD_CHAR)) {

            if (cmd.compare("help") == 0) {
                printf("There will be some help just right here !\n");
           } 

            else if (cmd.compare("exit") == 0) {
                quit = true;
            }

            else if (cmd.compare("clear") == 0) {
                printf("Nettoyage...\n");
                system("clear");
            }
            
            else if (cmd.compare("logo") == 0) {
                affiche_logo();
            }

            else {
                string res;
                res = argToString(cmd);
                if (!res.empty()) {
                    cout << "convert -> " << res << endl;
                    // Appeler fct qui va envoyer vers un noeud de calcul disponible
                    traitement_calcul(res);
                }
                res.clear();
            }
        }
        // Pas de commande à rallonge
        else if (cmd.size() > MAX_CMD_CHAR) {
            cerr << "Erreur: La commande est trop longue !" << endl;
        }

        cmd.erase();
        
    } while (!quit);

}

void usage(string prog_name) {
    cerr << "USAGE: " << prog_name << " <cmd> || <opérande>...(<a>,<b>,...)" << endl;
}


int main (int argc, char **argv) {
    openTerm();

    return 0;
}