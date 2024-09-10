#ifndef MAIN_H
#define MAIN_H

//PROGRAMME: 247-616 L-0002
//DESCRIPTION: Programme qui permet de tester le module interfaceMalyan
//Le programme permet de tester la communication avec une imprimante 3d
//de type MALYAN M200.
//Appuyer sur la touche:
//6: fait tourner le ventilateur (commande M106)
//7: arrête le ventilateur (commande M107)
//x: x transmet une commande inconnue (commande x000)
//Q: termine le programme
//Si la commande est complétée, l'imprimante retourne:
//  ok N0 P15 B15\n
//Si la commande est inconnue, l'imprimante retourne:
//  echo:Unknown command: "x000"\n
//  ok N0 P15 B15\n
//Pour plus de détails a propos de la configuration du port USB/Serie:
//https://en.wikibooks.org/wiki/Serial_Programming/termios

//HISTORIQUE:
// 2018-10-11, Yves Roy: creation
// 2020-09-06, Yves Roy: reception amelioree

#include <stdlib.h>
#include <stdio.h>

//DEFINITIONS REQUISES PAR LE PROGRAMME:

//Dependances materielles:
//pas de dependances materielles

//Dependances logicielles:
#define PILOTESERIEUSB_BAUDRATE_AVEC_B_AU_DEBUT   B115200

//INFORMATION PUBLIQUE:
//Definitions publiques:
//pas de definitions publiques
extern const char *devices[];
//Fonctions publiques:
void TogglePump(void);
float calculerDistanceEuclidienne(int x1, int y1, int x2, int y2);
float FonctionDistance(void);  // Déclaration de la fonction FonctionDistance
float lireDistanceCapteur(int fd);  // Déclaration de la fonction lireDistanceCapteur
int main(void);
void activerPompe(void);
void desactiverPompe(void);
void deplacerBras(int x, int y, int z);
int attendreFinMouvement(void);
void afficherPosition(void);
int trouverEtPrendreBloc(int fd);
void deplacerBrasEtAttendre(int x, int y, int z);
char getCh(void);
int attendrePositionEtScan(int x, int y, int z, int fd);
void Lit_Balance(void);
//Variables publiques:
//pas de variables publiques

#endif
