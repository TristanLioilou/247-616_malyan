#include "main.h"
#include "piloteSerieUSB.h"
#include "interfaceMalyan.h"
#include <string.h>

#define INTERFACEMALYAN_LONGUEUR_MAXIMALE_DES_COMMANDES 64
#define INTERFACEMALYAN_LONGUEUR_MAXIMALE_DES_REPONSES 64

// Déclarations de fonctions privées


// Variables privées
unsigned char interfaceMalyan_commande[INTERFACEMALYAN_LONGUEUR_MAXIMALE_DES_COMMANDES];
unsigned char interfaceMalyan_reponse[INTERFACEMALYAN_LONGUEUR_MAXIMALE_DES_REPONSES];

// Fonctions privées
int interfaceMalyan_ecritUneCommande(char *Commande, unsigned char Longueur)
{
    int retour;
    retour = piloteSerieUSB_ecrit(Commande, Longueur);
    if (retour != (int)Longueur)
    {
        return -1;
    }
    piloteSerieUSB_attendLaFinDeLEcriture();
    return retour;
}

int interfaceMalyan_recoitUneReponse(char *Reponse, unsigned char LongueurMaximale)
{
    return piloteSerieUSB_lit(Reponse, LongueurMaximale);
}

// Fonctions publiques
int interfaceMalyan_initialise(void)
{
    return 0;
}

int interfaceMalyan_termine(void)
{
    return 0;
}

// Commande pour récupérer la position actuelle
int interfaceMalyan_donneLaPosition(void)
{
    return interfaceMalyan_ecritUneCommande("M114\n", 5);  // M114 pour récupérer la position actuelle
}

// Fonction pour lire la réponse de la position
int interfaceMalyan_litLaPosition(char *Reponse, unsigned char LongueurMaximale)
{
    return piloteSerieUSB_lit(Reponse, LongueurMaximale);
}
