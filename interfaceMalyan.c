#include "main.h"
#include "piloteSerieUSB.h"
#include "interfaceMalyan.h"


#define INTERFACEMALYAN_LONGUEUR_MAXIMALE_DES_COMMANDES 64
#define INTERFACEMALYAN_LONGUEUR_MAXIMALE_DES_REPONSES 64

// Déclarations de fonctions privées


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

int interfaceMalyan_verifieMouvement(char *Reponse, unsigned char LongueurMaximale)
{
    // Envoyer la commande M2200 pour vérifier si le bras est en mouvement
    if (interfaceMalyan_ecritUneCommande("M2200\n", 6) < 0)
    {
        return -1;  // Erreur lors de l'envoi de la commande
    }

    // Lire la réponse
    return interfaceMalyan_recoitUneReponse(Reponse, LongueurMaximale);
}