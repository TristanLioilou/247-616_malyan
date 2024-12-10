#ifndef INTERFACETOUCHE_H
#define INTERFACETOUCHE_H

// MODULE: interfaceTouche
// DESCRIPTION: Module qui permet de lire les touches d'un clavier
// relie a un Beaglebone Bleu par un câble USB.
// Le module permet de configurer le clavier (stdin) en mode "raw" qui implique
// qu'il n'y a plus de traitement des touches fait par le BB-B. Il permet aussi
// de remettre le clavier dans le mode qui prevalait avant le changement de mode
// Le module est derive du programme L-0001uneTouche.
// interfaceTouche_initialise: permet de passer en mode "raw"
// interfaceTouche_lit: lit une valeur de touche frappée
// interfaceTouche_termine: permet de restaurer la configuration qui etait en
//   place au moment du dernier appel de la fonction interfaceTouche_initialise.

// ATTENTION: il est obligatoire qu'un programme qui fait appel à la fonction
// interfaceTouche_initialise pour pouvoir accéder aux touches du clavier fasse
// aussi appel à la fonction interface_Touche avant de se terminer pour éviter
// que le clavier demeure en mode "raw" apres la fin de l'execution de ce
// programme.
// Note: le changement de mode ne permet plus de reconnaître la commande CTRL+c
// pour eviter de voir un programme se terminer avant qu'il n'y ait eu appel a la
// fonction interfaceTouche_restaure.

// HISTORIQUE:
//  2018-10-11, Yves Roy, creation

// DEFINITIONS REQUISES PAR LE MODULE:
// Dependances materielles
//(copiez et adaptez ce qui suit dans "main.h")
// pas de dependances materielles

// Dependances logicielles
//(copiez et adaptez ce qui suit dans "main.h")
// pas de dependances logicielles

// INFORMATION PUBLIQUE:
// Definitions publiques:
// pas de definitions publiques

// Fonctions publiques:
int interfaceTouche_initialise(void);
int interfaceTouche_termine(void);
unsigned char interfaceTouche_lit(void);

// Variables publiques:
// pas de variables publiques

#endif
