#ifndef INTERFACEMALYAN_H
#define INTERFACEMALYAN_H

// Initialisation et terminaison de l'interface Malyan
int interfaceMalyan_initialise(void);
int interfaceMalyan_termine(void);

// Commandes pour le bras robotique
int interfaceMalyan_donneLaPosition(void);  // Commande pour demander la position
int interfaceMalyan_ecritUneCommande(char *Commande, unsigned char Longueur);
int interfaceMalyan_litLaPosition(char *Reponse, unsigned char LongueurMaximale);  // Fonction pour lire la position du bras
int interfaceMalyan_recoitUneReponse(char *Reponse, unsigned char LongueurMaximale);
#endif
