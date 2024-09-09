#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "piloteSerieUSB.h"
#include "interfaceMalyan.h"
#include "main.h"
#include "interfaceDistance.h"

#define MAX_LONGUEUR_COMMANDE 64
#define MAX_LONGUEUR_REPONSE 128

// Position actuelle du bras robotique
int posX = 0, posY = 0, posZ = 0;
int pompeActive = 0; // 0 : Pompe désactivée, 1 : Pompe activée

// Déclarations des fonctions utilisées

// Fonction pour envoyer une commande au bras robotique
int envoyerCommande(char* commande) {
    return interfaceMalyan_ecritUneCommande(commande, strlen(commande));
}

// Fonction pour déplacer le bras robotique aux coordonnées spécifiées par l'utilisateur
void deplacerBras(int x, int y, int z) {
    char commande[MAX_LONGUEUR_COMMANDE];

    // Création de la commande G-code pour se déplacer aux coordonnées (X, Y, Z)
    snprintf(commande, MAX_LONGUEUR_COMMANDE, "G0 X%d Y%d Z%d F10000\n", x, y, z);

    // Envoi de la commande
    if (envoyerCommande(commande) < 0) {
        printf("Erreur lors de l'envoi de la commande : %s\n", commande);
    } else {
        printf("Commande envoyée : %s\n", commande);
    }

    // Mise à jour de la position après le déplacement
    posX = x;
    posY = y;
    posZ = z;
}

// Fonction pour attendre que le bras ait terminé son mouvement
int attendreFinMouvement() {
    char reponse[MAX_LONGUEUR_REPONSE];

    // Demander la position actuelle du bras
    if (interfaceMalyan_donneLaPosition() < 0) {
        return 0;
    }

    // Lire la réponse qui peut être quelque chose comme "ok" ou une position
    if (interfaceMalyan_litLaPosition(reponse, MAX_LONGUEUR_REPONSE) < 0) {
        return 0;
    }

    // Vérifier si la réponse indique que le bras a terminé son mouvement
    printf("Réponse de position du bras : %s\n", reponse);

    // Si la réponse indique "ok" ou que le bras a terminé, retournez 1
    if (strstr(reponse, "ok") != NULL) {
        return 1;  // Mouvement terminé
    }

    return 0;  // Mouvement non terminé
}

// Fonction pour afficher la position actuelle du bras
void afficherPosition() {
    FonctionDistance();
    printf("Position actuelle du bras : X=%d Y=%d Z=%d\n", posX, posY, posZ);
}

// Fonction pour activer la pompe
void activerPompe() {
    char commande[MAX_LONGUEUR_COMMANDE];
    snprintf(commande, MAX_LONGUEUR_COMMANDE, "M2231 V1\n");

    // Envoi de la commande pour activer la pompe
    if (envoyerCommande(commande) < 0) {
        printf("Erreur lors de l'activation de la pompe.\n");
    } else {
        printf("Pompe activée.\n");
    }
}

// Fonction pour désactiver la pompe
void desactiverPompe() {
    char commande[MAX_LONGUEUR_COMMANDE];
    snprintf(commande, MAX_LONGUEUR_COMMANDE, "M2231 V0\n");

    // Envoi de la commande pour désactiver la pompe
    if (envoyerCommande(commande) < 0) {
        printf("Erreur lors de la désactivation de la pompe.\n");
    } else {
        printf("Pompe désactivée.\n");
    }
}

// Fonction pour lire une touche sans appuyer sur Enter
char getCh() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);  // Sauvegarde des paramètres du terminal
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // Désactive le mode canonique et l'écho
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Applique les nouveaux paramètres
    ch = getchar();  // Lit un caractère
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restaure les paramètres du terminal
    return ch;
}

// Fonction principale
int main() {
    char choix;

    // Initialisation de la communication série avec le bras robot
    if (piloteSerieUSB_initialise() != 0) {
        printf("Erreur d'initialisation de la communication série.\n");
        return EXIT_FAILURE;
    }

    if (interfaceMalyan_initialise() != 0) {
        printf("Erreur d'initialisation de l'interface Malyan.\n");
        return EXIT_FAILURE;
    }

    // Boucle principale pour contrôler le bras avec les touches
    while (1) {
        // Demander à l'utilisateur son choix d'action
        printf("\nQue voulez-vous faire ?\n");
        printf("Entrez 'I' pour entrer en mode manuel, 'P' pour afficher la position du bras, 'M' pour suivre une séquence de mouvement, 'E' pour basculer la pompe, ou 'Q' pour quitter :\n ");
                    printf("T : Capteur de distance\n");
        choix = getCh();  // Utilise getCh au lieu de getchar

        if (choix == 'Q' || choix == 'q') {
            printf("Arrêt du programme.\n");
            break;
        } else if (choix == 'P' || choix == 'p') {
            afficherPosition();
        } else if (choix == 'T' || choix == 't') {
            while((choix == 'T' || choix == 't'))
            {
                    FonctionDistance();
                     choix = getCh();  // Utilise getCh au lieu de getchar
            }
        } else if (choix == 'M' || choix == 'm') {
            // Début de la séquence de mouvements
            printf("Début de la séquence de mouvements :\n");
            while (getCh())
            {
                        
            deplacerBras(150, 10, 40);
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

            deplacerBras(150, 10, 15);
            // Activer la pompe après le premier mouvement
            activerPompe();
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

            // Déplacer le bras à la prochaine position
            deplacerBras(300, 10, 70);
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

            // Dernier mouvement, désactiver la pompe
            deplacerBras(300, 10, 20);
            desactiverPompe(); // Désactiver la pompe après le dernier mouvement
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

            // Finalement, revenir à la position initiale
            deplacerBras(300, 10, 70);
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }
            
            sleep(5);

            deplacerBras(300, 10, 15);
            activerPompe(); // Désactiver la pompe après le dernier mouvement
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

             deplacerBras(300, 10, 70);
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

            deplacerBras(300, -120, 70);
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

             deplacerBras(300, -120, 15);
             desactiverPompe(); 
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }

             deplacerBras(300, -120, 70);
            while (!attendreFinMouvement()) {
                usleep(100000); // Attendre un peu avant de vérifier à nouveau
            }
            }

        } else if (choix == 'I' || choix == 'i') {
            // Entrer en mode manuel
            printf("Mode manuel activé. Utilisez les touches suivantes pour déplacer le bras :\n");
            printf("W : Augmenter Y de 10\n");
            printf("S : Réduire Y de 10\n");
            printf("A : Réduire X de 10\n");
            printf("D : Augmenter X de 10\n");
            printf("Z : Augmenter Z de 10\n");
            printf("X : Réduire Z de 10\n");
            printf("O : Déplacer le bras à la position (150, 50, 0)\n");
            printf("Entrez 'Q' pour quitter le mode manuel.\n");

            // Mode manuel
            while (1) {
                choix = getCh();  // Utilise getCh au lieu de getchar()

                if (choix == 'Q' || choix == 'q') {
                    printf("Quitter le mode manuel.\n");
                    break;  // Sortir du mode manuel
                } else if (choix == 'W' || choix == 'w') {
                    posY -= 10;  // Augmenter Y de 10
                    deplacerBras(posX, posY, posZ);
                } else if (choix == 'S' || choix == 's') {
                    posY += 10;  // Réduire Y de 10
                    deplacerBras(posX, posY, posZ);
                } else if (choix == 'A' || choix == 'a') {
                    posX += 10;  // Réduire X de 10
                    deplacerBras(posX, posY, posZ);
                } else if (choix == 'D' || choix == 'd') {
                    posX -= 10;  // Augmenter X de 10
                    deplacerBras(posX, posY, posZ);
                } else if (choix == 'Z' || choix == 'z') {
                    posZ += 10;  // Augmenter Z de 10
                    deplacerBras(posX, posY, posZ);
                } else if (choix == 'X' || choix == 'x') {
                    posZ -= 10;  // Réduire Z de 10
                    deplacerBras(posX, posY, posZ);
                } else if (choix == 'O' || choix == 'o') {
                    posX = 150;
                    posY = 50;
                    posZ = 0;
                    deplacerBras(posX, posY, posZ);  // Déplacer à (150, 50, 0)
                } else if (choix == 'E' || choix == 'e') {
                    // Toggle de la pompe
                    if (pompeActive == 0) {
                        activerPompe();
                        pompeActive = 1;
                    } else {
                        desactiverPompe();
                        pompeActive = 0;
                    }
                } else {
                    printf("Commande inconnue. Essayez de nouveau.\n");
                }

                // Afficher la position actuelle du bras après chaque mouvement
                afficherPosition();
            }
        }
    }

    return EXIT_SUCCESS;
}
