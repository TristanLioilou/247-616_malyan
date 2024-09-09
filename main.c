//main:
// 2018-10-11, Yves Roy, creation (247-637 S-0003)

//INCLUSIONS
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "main.h"
#include "piloteSerieUSB.h"
#include "interfaceTouche.h"
#include "interfaceMalyan.h"

//Definitions privees
#define MAIN_LONGUEUR_MAXIMALE 99

//Declarations de fonctions privees:
int main_initialise(void);
void main_termine(void);

//Definitions de variables privees:
pid_t pid_fils; // PID du processus fils

//Definitions de fonctions privees:
int main_initialise(void)
{
    if (piloteSerieUSB_initialise() != 0)
    {
        return -1;
    }
    if (interfaceTouche_initialise() != 0)
    {
        return -1;
    }
    if (interfaceMalyan_initialise() != 0)
    {
        return -1;
    }
    return 0;
}

void main_termine(void)
{
    piloteSerieUSB_termine();
    interfaceTouche_termine();
    interfaceMalyan_termine();
}

// Fonction pour le processus fils
void processus_fils() {
    while (1) {
        pause(); // Attendre un signal pour agir
    }
}

//Definitions de variables publiques:
//pas de variables publiques

//Definitions de fonctions publiques:
int main(int argc,char** argv)
{
    int erreur = 0;
    unsigned char toucheLue = 'D';
    char reponse[MAIN_LONGUEUR_MAXIMALE + 1];
    int nombre;

    if (main_initialise())
    {
        printf("main_initialise: erreur\n");
        return 0;
    }

    // Création du processus fils
    pid_fils = fork();
    if (pid_fils < 0) {
        perror("Erreur lors du fork");
        return EXIT_FAILURE;
    } else if (pid_fils == 0) {
        // Code du processus fils
        processus_fils();
        exit(0);
    }

    fprintf(stdout, "Tapez:\n\r");
    fprintf(stdout, "Q\": pour terminer.\n\r");
    fprintf(stdout, "6\": pour démarrer le ventilateur.\n\r");
    fprintf(stdout, "7\": pour arrêter le ventilateur.\n\r");
    fprintf(stdout, "8\": donne la position actuelle.\n\r");
    fprintf(stdout, "P\": va à la position x=20, y=20, z=20.\n\r");
    fprintf(stdout, "H\": positionne la tête d'impression à l'origine (home).\n\r");
    fprintf(stdout, "autre chose pour générer une erreur.\n\r");
    fflush(stdout);
    
    while (toucheLue != 'Q')
    {
        printf("Entrez une commande\n");
        toucheLue = interfaceTouche_lit();
        printf("Caractère lu = '%c'\n", toucheLue);
        switch (toucheLue)
        {
            case '6':
                if (interfaceMalyan_demarreLeVentilateur() < 0)
                {
                    erreur = 1;
                }
                break;
            case '7':
                if (interfaceMalyan_arreteLeVentilateur() < 0)
                {
                    erreur = 1;
                }
                break;
            case '8':
                if (interfaceMalyan_donneLaPosition() < 0)
                {
                    erreur = 1;
                }
                break;
            case 'H':
                if (interfaceMalyan_vaALaPosition(20, 20, 20) < 0)
                {
                    erreur = 1;
                }
                break;
            case 'P':
                if (interfaceMalyan_retourneALaMaison() < 0)
                {
                    erreur = 1;
                }
                break;
            default:
                if (interfaceMalyan_genereUneErreur() < 0)
                {
                    erreur = 1;
                }
        }

        if (erreur == 1)
        {
            printf("erreur lors de la gestion de la commande\n");
            break;
        }
        else
        {
            usleep(100000);                
            nombre = interfaceMalyan_recoitUneReponse(reponse, MAIN_LONGUEUR_MAXIMALE);
            if (nombre < 0)
            {
                erreur = errno;
                printf("main: erreur lors de la lecture: %d\n", erreur);
                perror("erreur: ");
            }
            else
            {
                reponse[nombre] = '\0';
                printf("nombre reçu: %d, réponse: %s", nombre, reponse);      
//              fflush(stdout);
            }
        }
    }

    // Arrêter le processus fils avant de quitter
    kill(pid_fils, SIGTERM);
    wait(NULL); // Attendre que le fils se termine
    main_termine();
    return EXIT_SUCCESS;
}
