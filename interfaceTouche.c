// interfaceTouche
// Historique:
// 2018-11-12, Yves Roy, creation

// INCLUSIONS
#include <termios.h>
#include <errno.h>
#include "main.h"
#include "interfaceTouche.h"

// Definitions privees
// pas de definitions privees

// Declarations de fonctions privees:
// pas de fonctions privees

// Definitions de variables privees:
struct termios interfaceTouche_configurationDeDepart;

// Definitions de fonctions privees:
// pas de fonctions privees

// Definitions de variables publiques:
// pas de variables publiques

// Definitions de fonctions publiques:
int interfaceTouche_initialise(void)
{
    int fichier;
    struct termios modeRaw;

    if (setvbuf(stdin, NULL, _IONBF, 0)) // pas de sauvegarde temporaire (buffer)
    {
        return -1;
    }
    fflush(stdin);

    fichier = fileno(stdin);
    if (fichier == -1)
    {
        return -1;
    }
    tcflush(fichier, TCIOFLUSH);

    if (tcgetattr(fichier, &interfaceTouche_configurationDeDepart))
    {
        return -1;
    }

    modeRaw = interfaceTouche_configurationDeDepart;

    modeRaw.c_iflag |= IGNBRK;                  // pas de traitement préalable de CTRL-C pour éviter les sorties prématurées
    modeRaw.c_iflag &= ~BRKINT;                 // pas de génération de "signal" INT lorsqu'un "break" est détecté
    modeRaw.c_cflag |= CREAD;                   // pour permettre la lecture de données
    modeRaw.c_lflag &= ~(ISIG | ECHO | ICANON); // pas de touches speciales, echo et canonique
    modeRaw.c_cc[VMIN] = 1;                     // pour attendre au moins 1 caractere en mode "raw"
    modeRaw.c_cc[VTIME] = 0;                    // pour ne pas avoir de "timeout" en mode "raw"

    if (tcsetattr(fichier, TCSAFLUSH, &modeRaw))
    {
        return -1;
    }
    return 0;
}

int interfaceTouche_termine(void)
{
    int fichier;
    int retour;

    fflush(stdin);
    fichier = fileno(stdin);
    if (fichier == -1)
    {
        return -1;
    }

    do
    {
        retour = tcflush(fichier, TCIOFLUSH);
    } while (retour == -1 && errno == EINTR);

    do
    {
        retour = tcsetattr(fichier, TCSAFLUSH,
                           &interfaceTouche_configurationDeDepart);
    } while (retour == -1 && errno == EINTR);

    if (retour == -1)
    {
        return -1;
    }

    return 0;
}

unsigned char interfaceTouche_lit(void)
{
    return getchar();
}
