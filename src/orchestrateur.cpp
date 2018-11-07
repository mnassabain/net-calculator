#include "orchestrateur.h"

#define BUFFSIZE        1024
#define STDINFD         0
#define PORT            8000
#define TEMPS_ATTENTE   10

Node::Node() {
    command = "";
    operations = "";
    state = false;
}

Node::Node(struct sockaddr_in ad, int nb, string op) {
    addr = ad;
    nbArg = nb;
    state = true;
    operations = op;

    this->setState(true);

    this->lastHello = time(NULL);
}


/*
 * spec est de la forme "+:2" pour l'addition par exemple
 */
Node::Node(struct sockaddr_in addr, string spec) { 
    char op;
    char c;
    int nbarg;

    stringstream stream(spec);

    // Décomposition de la chaine avec un stream
    stream >> op >> c >> nbarg;

    // Création du noeud
    this->nbArg = nbarg;
    this->operations.push_back(op);

    this->addr = addr;

    this->setState(true);

    this->lastHello = time(NULL);
}

struct sockaddr_in Node::getAddr(void) {
    return this->addr;
}

string Node::getOp(void) {
    return this->operations;
}

void Node::setCommand(string cmd) {
    this->command = cmd;
}

void Node::setState(bool newState) {
    this->state = newState;
}

bool Node::getState(void) { return this->state; }

string Node::getCommand() { return this->command; }

void Orchestrateur::afficheLogo(void) {
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

void Orchestrateur::gestionNoeud(struct sockaddr_in addr, string cmd) {
    Node n(addr, cmd);
    Node *res;
    if (((res = this->findNode(n)) != NULL)) {
       /* if (res->getState()) { // Le noeud est sensé être disponible...

            // On met à jour l'heure de son dernier hello
            res->lastHello = time(NULL);
        }
        else {
            // Fin d'un calcul !
            char *buf = (char *) malloc(BUFFSIZE);

            if (buf == NULL) {
               fprintf(stderr, "Error: impossible to allocate the buffer for reception\n");
                exit(EXIT_FAILURE);
            }

            res->setState(true);
            res->lastHello = time(NULL);

            cout << endl << endl << "@" << inet_ntoa(res->getAddr().sin_addr) << ":" << ntohs(res->getAddr().sin_port) << " -> Résultat du calcul " << res->getCommand() << " = " << cmd << endl;

            cout << endl << "orchestrateur> " << flush;
        } */

        if (cmd[0] == ':') { // C'est un résultat !
            // Fin d'un calcul !
            cmd.erase(cmd.begin());

            res->setState(true);
            res->lastHello = time(NULL);

            cout << endl << endl << "@" << inet_ntoa(res->getAddr().sin_addr) << ":" << ntohs(res->getAddr().sin_port) << " -> Résultat du calcul " << res->getCommand() << " = " << cmd << endl;

            cout << endl << "orchestrateur> " << flush;
        }
        else { // C'est un hello
            // On met à jour l'heure de son dernier hello
            res->lastHello = time(NULL);
        }
    }
    else {
        // Le noeud n'existe pas, on l'ajoute !
        this->nodeTab.push_back(n);
    }
}

string Orchestrateur::argToStr(string arg) {
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
        Orchestrateur::usage("./Orchestrateur");
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

        // S'il n'y a aucun argument...
        if (str.size() == nbOperande + 1) {
            cerr << "Argument incorrect" << endl;
            usage("./Orchestrateur");
            return "";
        }
        else if (c != ')') {
            cerr << "Parenthèse fermante manquante" << endl;
            usage("./Orchestrateur");
            return "";
        }

        return str;
    }
}

void Orchestrateur::usage(string pname) {
    cerr << "USAGE: " << pname << " <cmd> || <opérande>...(<a>,<b>,...)" << endl;
}

void Orchestrateur::addNode(Node n) {
    this->nodeTab.push_back(n);
}

int Orchestrateur::maxFd(void) {
    int max = 0;

    // On regarde le fd max 
    if (max < this->socketFd) {
        max = this->socketFd;
    }

    return max;
}

void Orchestrateur::initAddr(void) {

    // Initialisation de l'@
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    this->addr.sin_port = htons(PORT);

    this->port = PORT;

    // On crée un socket
    if((socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket ");
        exit(EXIT_FAILURE);
    }
    
    // On relie l'@ au socket
    if ((bind(socketFd, (struct sockaddr*) &addr, sizeof(addr))) == -1) {
		perror("bind ");
		exit(EXIT_FAILURE);
	}

    this->socketFd = socketFd;

    cout << "Hey my @ is " << inet_ntoa(this->addr.sin_addr) <<
    " and my port is " << ntohs(this->addr.sin_port) << endl;
}

Node* Orchestrateur::findNode(Node n) {
    int size = this->nodeTab.size();
    int i;
    Node *ptr = NULL;

    // parcours de la liste de noeuds de l'orchstrateur
    for(i=0 ; i<size ; i++) {
        if ((this->nodeTab[i].getAddr().sin_addr.s_addr == n.getAddr().sin_addr.s_addr)
        && (this->nodeTab[i].getAddr().sin_port == n.getAddr().sin_port)) {
            ptr = &this->nodeTab[i];
        }
    }

    return ptr;
}

void Orchestrateur::initFdSet(fd_set *set) {
    if (set == NULL) {
        fprintf(stderr, "Impossible d'initialiser un ensemble\n");
        exit(EXIT_FAILURE);
    }

    // Il ne faut pas oublier le fd de l'orch..
    FD_SET(this->socketFd, set);
    cout << this->socketFd << " added" << endl;

    // On rajoute l'entrée std
    FD_SET(STDINFD, set);
}

void Orchestrateur::updateAllNodes(void) {
    int i;
    int size = this->nodeTab.size();

    for(i=0 ; i<size ; i++) {
        time_t cur_time = time(NULL);
        
        // Si cela fait plus de 10 sec que nous n'avons pas reçu de nouvelles...
        if (((cur_time - this->nodeTab[i].lastHello) > TEMPS_ATTENTE)) {
            cout << "\nLe noeud calculant " << this->nodeTab[i].getOp() << " a été supprimé " << endl;
            this->nodeTab.erase(this->nodeTab.begin() + i);
            cout << "orchestrateur> " << flush;
        }
    }
}

void Orchestrateur::displayNodeVec(void) {
    int size = this->nodeTab.size();
    int i;

    cout << "\nListe des NOEUDS" << endl;

    cout << "******************************************************************\n" << endl;

    for(i=0 ; i<size ; i++) {
        cout << "@ = " << inet_ntoa(nodeTab[i].getAddr().sin_addr) << " ; Port = " << ntohs(nodeTab[i].getAddr().sin_port) << 
        " ; Operation = " << nodeTab[i].getOp() << " ; Time = " << time(NULL) - nodeTab[i].lastHello << endl;
    }

    if (size == 0) {
        cout << "La liste de noeud est vide pour le moment !" << endl;
        cout << "Pour ajouter un noeud -> @ = " << inet_ntoa(this->addr.sin_addr) << " Port -> " << this->port << endl;
    }

    cout << "\n******************************************************************\n" << endl;
}

/* ATTENTION
 * Précondition : cmd doit être de la bonne forme !
 */
void Orchestrateur::traiteCmd(string cmd, string calc) { 
    string op;
    int i = 0;

    while(cmd[i] != ':') {
        op.push_back(cmd[i]);
        i++;
    }

    int size = this->nodeTab.size();

    i = 0;

    while (i<size) {
        if ((op.compare(this->nodeTab[i].getOp()) == 0) && (this->nodeTab[i].getState())) { // On a trouvé le noeud
            if ((sendto(socketFd, cmd.c_str(), cmd.size(), 0, (struct sockaddr*) &(this->nodeTab[i].addr), sizeof(struct sockaddr_in))) < 0) {
		        perror("send message ");
		        exit(EXIT_FAILURE);
	        }

            this->nodeTab[i].setState(false);
            this->nodeTab[i].setCommand(calc);

            cout << "Calulating on " << inet_ntoa(this->nodeTab[i].getAddr().sin_addr) << ":" << ntohs(this->nodeTab[i].getAddr().sin_port) << endl;

            break;
        }
        i++;
    }

    if (i >= size) {
        cout << "Aucun noeud disponible pour calculer " << op << endl;
    }
}

// Routine de l'orchestrateur...
void Orchestrateur::openTerm(void) {

    Orchestrateur::afficheLogo();

    bool quit = false;
    int i;

    // Initialisation pour select 
    fd_set read_fds;
    fd_set read_fds_tmp;
    int retVal; // Val de retour de select
    unsigned char c; // Pour traitement des commandes de l'utilisateur  
    string buffer; // stocker une cmd de l'utilisateur
    string calc; // stocke un calcul sous la bonne syntaxe

    struct sockaddr_in addrFound;

/*******************************************************************************
 * ATTRIBUTION D'UNE @ POUR L'ORCHESTRATEUR *
 */

    // On initialise l'@ et le socket de l'orchestrateur 
    this->initAddr();

    FD_ZERO(&read_fds);
    this->initFdSet(&read_fds);

    cout << "orchestrateur> " << flush;

    char msg[BUFFSIZE];
    unsigned int addr_len = sizeof(struct sockaddr_in);

    struct timeval tv;
    struct timeval tv_tmp;

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    while (quit != true) {
        /* On fait un select pour gérer de manière 
         * non bloquante les données reçues...
         */
        read_fds_tmp = read_fds;
        tv_tmp = tv;
        retVal = select(this->maxFd()+1, &read_fds_tmp, 0, 0, &tv_tmp);

        if(retVal == -1) { // select a rencontré une erreur
            perror("select ");
            exit(EXIT_FAILURE);
        }
        else {
            if(FD_ISSET(STDINFD, &read_fds_tmp)) {
                buffer.erase();

                i = 0;
                
                while ((c = getchar()) != '\n') {
                    if (i>BUFFSIZE) {
                        // On refuse les commandes trop longues...
                        cerr << "La commande est trop longue !" << endl;
                        break;  
                    }

                    buffer.push_back(c);
                    i++;
                }

                if (i>=BUFFSIZE) {
                    buffer = "";
                }
                else {

                    if (buffer.compare("quit") == 0) {
                        quit = true; // On quitte le terminal
                        break;
                    }
                    else if (buffer.compare("clear") == 0) {
                        system("clear");
                    }
                    else if (buffer.compare("logo") == 0) {
                        Orchestrateur::afficheLogo();
                    }
                    else if (buffer.compare("nodes") == 0) {
                        Orchestrateur::displayNodeVec();
                    }
                    else {
                        calc = Orchestrateur::argToStr(buffer); // On donne une bonne syntaxe à la commande

                        if (calc != "") {
                            Orchestrateur::traiteCmd(calc, string(buffer));
                        }
                    }

                    
                }
                cout << "orchestrateur> " << flush;
            }
            else if (FD_ISSET(this->socketFd, &read_fds_tmp)) {
                if ((recvfrom(this->socketFd, msg, BUFFSIZE, 0, (struct sockaddr*) &addrFound, &addr_len) == -1)) {
					perror("message reception ");
					exit(EXIT_FAILURE);
				}
                // Comment est le hello d'un noued ? => +:4
                string spec(msg);

                this->gestionNoeud(addrFound, spec);
                
            }

            this->updateAllNodes();
        }
    }
}

int main (int argc, char **argv) {
   Orchestrateur orchest;
   orchest.openTerm();

    if (argc != 1) {
        orchest.usage(argv[0]);
    }

    return 0;  
}
