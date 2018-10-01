#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TRUE        1
#define FALSE       0
#define BUFFSIZE    1024

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

char *lireCmd (void) {
    char *res = calloc(256, 1);
    if (res == NULL) {
        fprintf(stderr, "Error: can't allocate the memory w/ calloc\n");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    char c;
    while ((c = getchar()) != '\n' && i<256) {
        res[i] = c;
        i++;
    }

    if (i>256) {
        fprintf(stderr, "La commande est trop longue !\n");
        free(res);
        res = NULL;
    }
    else {
        res[i] = '\0';
    }

    return res;
}

void openTerm (void) {

    affiche_logo();

    char *cmd = NULL;
    int quit = FALSE;
    

    do {
        if (cmd != NULL)
            free(cmd);

        printf("\norchestreur> ");
        cmd = lireCmd();

        if (strcmp(cmd, "help") == 0) {
            printf("There will be some help just right here !\n");
        } 

        else if (strcmp(cmd, "exit") == 0) {
            quit = TRUE;
        }

        else {
            fprintf(stderr, "Commande %s non recconue, veuillez entrez une"
                " autre commande\n", cmd);
        }
        
    } while (!quit);

    if (cmd != NULL)
        free(cmd);

}

int main (int argc, char **argv) {
    openTerm();

    exit(EXIT_SUCCESS);
}