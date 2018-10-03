#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

        int size_before = str.size();
    
        while(((i < size) && (c = arg.at(i)) != ')')) {
            if ((c >= '0') && (c <= '9')) {
            str.push_back(c);
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

        if (str.size() == size_before) {
            cerr << "Argument incorrect" << endl;
            usage("./Orchestrateur");
            return "";
        }


        return str;
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
                    cout << res << endl;
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

   /* string str = argToString("*+-/(4, 4, 8, 9, 4 4 4;)");

    if (!str.empty()) {
        cout << "Opérande(s) trouvée(s) : " << str << endl;
    }
    else {
        cout << "rien trouvé.." << endl;
    }*/

    return 0;
}