#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <fcntl.h>
#include "piloteSerieUSB.h"
#include "interfaceMalyan.h"
#include "main.h"
#include "interfaceDistance.h"
#include <math.h>
#include "interfaceBalance.h"

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
void deplacerBrasEtAttendre(int x, int y, int z) {
    char commande[MAX_LONGUEUR_COMMANDE];
    snprintf(commande, MAX_LONGUEUR_COMMANDE, "G0 X%d Y%d Z%d F2000\n", x, y, z);

    // Envoi de la commande
    if (envoyerCommande(commande) < 0) {
        printf("Erreur lors de l'envoi de la commande : %s\n", commande);
        return;
    }

    // Attendre la confirmation de fin de mouvement
    while (!attendreFinMouvement()) {
        usleep(100000);  // Attendre un peu avant de vérifier à nouveau
    }
}

// Fonction pour attendre que le bras ait terminé son mouvement
int attendreFinMouvement() {
    char reponse[MAX_LONGUEUR_REPONSE];

    // Envoyer la commande M2200 et récupérer la réponse
    if (interfaceMalyan_verifieMouvement(reponse, MAX_LONGUEUR_REPONSE) < 0) {
        return 0;  // En cas d'erreur, on retourne 0 pour indiquer un problème
    }

    // Vérifier si le bras est en mouvement (V1) ou arrêté (V0)
    printf("\nRéponse du bras : %s", reponse);

    if (strstr(reponse, "V0") != NULL) {
        return 1;  // Mouvement terminé
    } else if (strstr(reponse, "V1") != NULL) {
        return 0;  // Bras en mouvement
    }

    return 0;  // Si la réponse ne correspond pas, on considère que le bras est en mouvement par défaut
}

// Fonction pour afficher la position actuelle du bras
void afficherPosition() {
    printf("Distance lue: %.2f cm\n",FonctionDistance());
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
    printf("\nQue voulez-vous faire ?\n");
    printf("Entrez 'I' pour entrer en mode manuel, T : Capteur de distance, 'P' pour afficher la position du bras, 'M' pour suivre une séquence de mouvement, 'E' pour basculer la pompe, ou 'Q' pour quitter :\n ");
    
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

float calculerDistanceEuclidienne(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

int trouverEtPrendreBloc(int fd) {
    // Définir la zone de scan
    int startX = 150, startY = 180, startZ = 160;  // Position de départ
    int endX = 110, endY = 280;                    // Position de fin
    int stepSize = 10;                              // Taille du pas pour les déplacements
    float distance;

    // Variables pour stocker les positions des blocs détectés
    int positionsX[100], positionsY[100];
    int count = 0;

    // Balayage de la zone
    printf("Démarrage du scan pour détecter le bloc...\n");
    for (int x = startX; x >= endX; x -= stepSize) {
        for (int y = startY; y <= endY; y += stepSize) {
            // Déplacer le bras à la position (x, y) et vérifier la distance
            deplacerBrasEtAttendre(x, y, startZ);

            // Attendre un peu pour stabiliser la position du bras
            while (!attendreFinMouvement()) {
                usleep(100000);  // Attendre 100ms avant de vérifier à nouveau
            }

            // Lire la distance à l'aide du capteur
            distance = FonctionDistance();  // Utilisation de fd
            if (distance < 0) {
                printf("Erreur lors de la lecture de la distance.\n");
                continue;  // Réessayer à la prochaine position si la lecture échoue
            }

            // Si la distance est inférieure à 11 cm, considérer cette position comme un bloc détecté
            if (distance < 11.0) {
                printf("Bloc détecté à : X=%d Y=%d Z=%f cm\n", x, y, distance);
                positionsX[count] = x;
                positionsY[count] = y;
                count++;
            }
        }
    }

    // Si aucun bloc n'a été détecté
    if (count == 0) {
        printf("Aucun bloc détecté dans la zone spécifiée.\n");
        
        return(0);
    }
    

    // Calculer la position moyenne (centrée) parmi tous les blocs détectés
    int centreX = 0, centreY = 0;
    for (int i = 0; i < count; i++) {
        centreX += positionsX[i];
        centreY += positionsY[i];
    }
    centreX /= count;
    centreY /= count;

    // Maintenant, trouver la position la plus proche du centre calculé
    int bestX = -1, bestY = -1;
    float minDistance = 1000.0f;  // Une distance initiale élevée

    for (int i = 0; i < count; i++) {
        float dist = calculerDistanceEuclidienne(positionsX[i], positionsY[i], centreX, centreY);
        if (dist < minDistance) {
            minDistance = dist;
            bestX = positionsX[i];
            bestY = positionsY[i];
        }
    }

    // Positionner le bras au-dessus du bloc le plus centré et activer la pompe
    if (bestX != -1 && bestY != -1) {
        printf("Déplacement vers le bloc centré à X=%d Y=%d\n", bestX, bestY);
        deplacerBrasEtAttendre(bestX + 15, bestY + 35, 105);  // Déplacer à X, Y-20, Z=105 (position de saisie)
        activerPompe();  // Activer la pompe pour saisir le bloc
        printf("Pompe activée pour prendre le bloc à X=%d Y=%d\n", bestX, bestY);
    } else {
        printf("Aucun bloc valide trouvé pour la prise.\n");
        return(0);
    }
    return(1);
}



int attendrePositionEtScan(int x, int y, int z, int fd) {
    // Déplacer le bras à la position spécifiée
    deplacerBrasEtAttendre(x, y, z);

    // Attendre que le bras ait atteint la position cible
    printf("Attente de la fin du mouvement à la position X=%d Y=%d Z=%d...\n", x, y, z);
    while (!attendreFinMouvement()) {
        usleep(1000000);  // Attendre un peu avant de vérifier à nouveau
    }

    // Vérification du capteur de distance avant de procéder au scan
    float distanceTest = FonctionDistance();
    if (distanceTest < 0) {
        printf("Erreur lors de la lecture du capteur de distance avant le scan.\n");
        return 0 ;  // Ne pas procéder si le capteur ne fonctionne pas
    }

    // Si tout va bien, commencer le scan
    printf("Mouvement terminé, début du scan...\n");
    return (trouverEtPrendreBloc(fd));  // Appeler la fonction pour démarrer le scan
}

void TogglePump()
{
static int pompeActive = 0;
    if (pompeActive) 
    {
        desactiverPompe();
        pompeActive = 0;  // Pompe désactivée
        printf("Pompe désactivée.\n");
    }
    else 
    {
        activerPompe();
        pompeActive = 1;  // Pompe activée
        printf("Pompe activée.\n");
    }   
}
void Lit_Balance()
{
char test_command[] = "T<cr><lf>"; // Commande de tare
char Value[255];
int port_index_balance = detect_port();  // Détecter le port de la balance
    if (port_index_balance == -1) {
    fprintf(stderr, "Balance non détectée sur les ports disponibles.\n");
    return ;
    }
 const char *port_name_balance = devices[port_index_balance];
    int fd_balance = configure_port(port_name_balance);  // Configurer la balance
    if (fd_balance == -1) {
    fprintf(stderr, "Impossible de configurer la balance.\n");
    return ;
    }
    send_command(fd_balance, test_command);
  if (read_response(fd_balance, Value, sizeof(Value)) == 0) {
     printf("Poids lu : %s\n", Value);
     } else {
          printf("Erreur de lecture de la balance.\n");
     }
close(fd_balance);
}

// Fonction principale
int main() {
    char choix;
    int fd;

      // Descripteur de fichier pour le capteur de distance
    // Initialisation de la communication série avec le bras robot
    if (piloteSerieUSB_initialise() != 0) {
        printf("Erreur d'initialisation de la communication série.\n");
        return EXIT_FAILURE;
    }

    if (interfaceMalyan_initialise() != 0) {
        printf("Erreur d'initialisation de l'interface Malyan.\n");
        return EXIT_FAILURE;
    }

    // Ouverture du capteur de distance (ajustez selon votre configuration)
    fd = open("/dev/i2c-1", O_RDWR);  // Exemple de périphérique I2C, ajustez selon votre système
    if (fd == -1) {
        perror("Erreur d'ouverture du périphérique I2C");
        return EXIT_FAILURE;
    }

      
    
    // Boucle principale pour contrôler le bras avec les touches
    while (1) {
        // Demander à l'utilisateur son choix d'action
        choix = getCh();  // Utilise getCh au lieu de getchar()
   
        if (choix == 'Q' || choix == 'q') {
            printf("Arrêt du programme.\n");
            break;
        } else if (choix == 'P' || choix == 'p') {
            afficherPosition();
         } else if (choix == 'E' || choix == 'e') {
            TogglePump();
        } else if (choix == 'T' || choix == 't') {
            while((choix == 'T' || choix == 't')) {
                printf("Distance lue: %.2f cm\n",FonctionDistance());  // Lire et afficher la distance
                choix = getCh();  // Utilise getCh au lieu de getchar
            }
        } else if (choix == 'M' || choix == 'm') {
            // Début de la séquence de mouvements
            printf("Début de la séquence de mouvements :\n");
            //deplacerBrasEtAttendre(120, 100, 160);  // Mouvement 0
            deplacerBrasEtAttendre(120, 230, 160);  // Mouvement 1
            
           // if(attendrePositionEtScan(150, 240, 160, fd))
            //{
            activerPompe();
            deplacerBrasEtAttendre(150, 240, 160);  // Mouvement 3
            deplacerBrasEtAttendre(230, -160, 160);
            deplacerBrasEtAttendre(230, -160, 100);
            desactiverPompe();
            deplacerBrasEtAttendre(230, -160, 160);
            Lit_Balance();
            deplacerBrasEtAttendre(230, -160, 100);
            activerPompe();
            deplacerBrasEtAttendre(230, -160, 160);
            deplacerBrasEtAttendre(70, -230, 160);
            deplacerBrasEtAttendre(70, -230, 20);
            desactiverPompe();
            deplacerBrasEtAttendre(70, -230, 160);
            
            //choix = getCh();
           // }
            deplacerBrasEtAttendre(150, 240, 160);
        } else if (choix == 'I' || choix == 'i') {
            // Entrer en mode manuel et déplacer le bras
            while (1) {
                printf("Déplacez le bras (WASD pour X/Y, Z/X pour Z, O pour position initiale, Q pour quitter) :\n");
                choix = getCh();  // Utilise getCh au lieu de getchar
                afficherPosition();
                if (choix == 'Q' || choix == 'q') {
                    break;
                } else if (choix == 'W' || choix == 'w') {
                    posY -= 10;  // Réduire Y de 10
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'S' || choix == 's') {
                    posY += 10;  // Réduire Y de 10
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'A' || choix == 'a') {
                    posX += 10;  // Réduire X de 10
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'D' || choix == 'd') {
                    posX -= 10;  // Augmenter X de 10
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'Z' || choix == 'z') {
                    posZ += 10;  // Augmenter Z de 10
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'X' || choix == 'x') {
                    posZ -= 10;  // Réduire Z de 10
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'O' || choix == 'o') {
                    posX = 150; posY = 50; posZ = 0;  // Déplacement vers la position (150, 50, 0)
                    deplacerBrasEtAttendre(posX, posY, posZ);
                } else if (choix == 'E' || choix == 'e') {
                    TogglePump();// Toggle de l'état de la pompe
                }
              }
         }
    }
    // Fermeture des ressources et nettoyage
    interfaceMalyan_termine();
    piloteSerieUSB_termine();
    close(fd);  // Fermer le descripteur de fichier du capteur de distance
    //close(fd_balance);
    return EXIT_SUCCESS;
}
