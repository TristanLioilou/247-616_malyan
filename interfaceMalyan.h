#ifndef INTERFACEMALYAN_H
#define INTERFACEMALYAN_H

//MODULE: interfaceMalyan
//DESCRIPTION: Module qui permet d'envoyer des commandes a une imprimante 3D
//de marque Malyan (M200) et de recevoir les reponses a ces commandes.

//HISTORIQUE:
// 2018-10-11, Yves Roy, creation

//DEFINITIONS REQUISES PAR LE MODULE:
//Dependances materielles
//(copiez et adaptez ce qui suit dans "main.h")
//pas de dependances materielles

//Dependances logicielles
//(copiez et adaptez ce qui suit dans "main.h")
//pas de dependances logicielles

//INFORMATION PUBLIQUE:
//Definitions publiques:
//pas de definitions publiques

//Fonctions publiques:
int interfaceMalyan_initialise(void);
int interfaceMalyan_termine(void);
int interfaceMalyan_demarreLeVentilateur(void);
int interfaceMalyan_arreteLeVentilateur(void);
int interfaceMalyan_genereUneErreur(void);
int interfaceMalyan_recoitUneReponse(char *Reponse, unsigned char LongueurMaximale);

int interfaceMalyan_demarreLaPompeDuUARM(void);
int interfaceMalyan_arreteLaPompeDuUARM(void);

int interfaceMalyan_donneLaPosition(void) ;
int interfaceMalyan_vaALaPosition(int x, int y, int z) ;
int interfaceMalyan_retourneALaMaison(void) ;

//Variables publiques:
//pas de variables publiques

#endif
