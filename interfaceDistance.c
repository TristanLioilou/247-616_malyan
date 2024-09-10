#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "interfaceDistance.h"
#include "main.h"

// Définir les macros nécessaires
#define MAX_TENTATIVES 100  // Nombre maximal de tentatives

// Fonction principale pour lire la distance
float FonctionDistance(void) {
    int fdPortI2C = open(I2C_FICHIER, O_RDWR);
    if (fdPortI2C == -1) {
        printf("Erreur: I2C initialisation step 1 - %s\n", strerror(errno));
        return -1;  // Retourner une valeur d'erreur
    }

    // Sélectionner le périphérique I2C
    if (ioctl(fdPortI2C, I2C_SLAVE_FORCE, I2C_ADRESSE) < 0) {
        printf("Erreur: I2C initialisation step 2 - %s\n", strerror(errno));
        close(fdPortI2C);
        return -1;  // Retourner une valeur d'erreur
    }

    // Configurer le capteur
    if (interfaceVL6180x_configure(fdPortI2C) < 0) {
        close(fdPortI2C);
        return -1;  // Retourner une valeur d'erreur
    }

    // Essayer de lire la distance jusqu'à ce que cela réussisse
    float distance = -1;
    int tentative = 0;
    while (distance < 0 && tentative < MAX_TENTATIVES) {
        distance = lireDistanceCapteur(fdPortI2C);  // Lire la distance
        if (distance < 0) {
            printf("Tentative %d échouée. Nouvelle tentative...\n", tentative + 1);
            tentative++;
            usleep(500000);  // Attendre 500ms avant de réessayer
        }
    }

    // Vérifier si la lecture a réussi après les tentatives
    if (distance < 0) {
        printf("Erreur: Lecture de la distance échouée après %d tentatives.\n", MAX_TENTATIVES);
        close(fdPortI2C);
        return -1;  // Retourner une valeur d'erreur si toutes les tentatives échouent
    }

    // Fermer le descripteur I2C
    close(fdPortI2C);
    
    return distance;  // Retourner la distance lue
}

// Fonction pour écrire dans le capteur VL6180x
int interfaceVL6180x_ecrit(int fd, uint16_t Registre, uint8_t Donnee) {
    uint8_t message[3];
    message[0] = (uint8_t)(Registre >> 8);
    message[1] = (uint8_t)(Registre & 0xFF);
    message[2] = Donnee;

    if (write(fd, message, 3) != 3) {
        printf("Erreur: interfaceVL6180x_ecrit - %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

// Fonction pour lire une valeur du capteur VL6180x
int interfaceVL6180x_lit(int fd, uint16_t Registre, uint8_t *Donnee) {
    uint8_t Commande[2];
    Commande[0] = (uint8_t)(Registre >> 8);
    Commande[1] = (uint8_t)(Registre & 0xFF);

    if (write(fd, Commande, 2) != 2) {
        printf("Erreur: interfaceVL6180x_lit (écriture) - %s\n", strerror(errno));
        return -1;
    }

    if (read(fd, Donnee, 1) != 1) {
        printf("Erreur: interfaceVL6180x_lit (lecture) - %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

// Fonction pour configurer le capteur VL6180x
int interfaceVL6180x_configure(int fd) {
    uint16_t reg_settings[][2] = {
        {0x0207, 0x01}, {0x0208, 0x01}, {0x0096, 0x00}, {0x0097, 0xfd},
        {0x00e3, 0x00}, {0x00e4, 0x04}, {0x00e5, 0x02}, {0x00e6, 0x01},
        {0x00e7, 0x03}, {0x00f5, 0x02}, {0x00d9, 0x05}, {0x00db, 0xce},
        {0x00dc, 0x03}, {0x00dd, 0xf8}, {0x009f, 0x00}, {0x00a3, 0x3c},
        {0x00b7, 0x00}, {0x00bb, 0x3c}, {0x00b2, 0x09}, {0x00ca, 0x09},
        {0x0198, 0x01}, {0x01b0, 0x17}, {0x01ad, 0x00}, {0x00ff, 0x05},
        {0x0100, 0x05}, {0x0199, 0x05}, {0x01a6, 0x1b}, {0x01ac, 0x3e},
        {0x01a7, 0x1f}, {0x0030, 0x00}, {0x0011, 0x10}, {0x010a, 0x30},
        {0x003f, 0x46}, {0x0031, 0xFF}, {0x0040, 0x63}, {0x002e, 0x01},
        {0x001b, 0x09}, {0x003e, 0x31}, {0x0014, 0x24}, {0x0016, 0x00}
    };

    for (size_t i = 0; i < sizeof(reg_settings) / sizeof(reg_settings[0]); i++) {
        if (interfaceVL6180x_ecrit(fd, reg_settings[i][0], reg_settings[i][1]) < 0) {
            return -1;
        }
    }
    return 0;
}

// Fonction pour lire la distance à partir du capteur
int interfaceVL6180x_litDistance(int fd, float *Distance) {
    uint8_t valeur;

    // Lire la distance
    if (interfaceVL6180x_lit(fd, 0x62, &valeur) < 0) {
        printf("Erreur: interfaceVL6180x_litDistance (lecture) - %s\n", strerror(errno));
        return -1;
    }

    *Distance = (float)valeur / 10.0; // Conversion en cm
    return 0;
}

// Fonction pour lire la distance avec le capteur VL6180x
float lireDistanceCapteur(int fd) {
    uint8_t valeur;
    float distance = -1;

    // Démarrer la mesure de distance
    if (interfaceVL6180x_ecrit(fd, 0x18, 0x01) < 0) {
        printf("Erreur: Demande de mesure de distance échouée.\n");
        return -1;
    }

    // Attendre que la mesure soit prête
    do {
        if (interfaceVL6180x_lit(fd, 0x4F, &valeur) < 0) {
            printf("Erreur: Lecture de l'état de mesure échouée.\n");
            usleep(1000000);  // Attendre un peu avant de réessayer
            return -1;
        }
        usleep(100000);  // Attendre 100 ms pour vérifier l'état
    } while ((valeur & 0x07) != 0x04);  // Vérifier que le bit de statut indique que la mesure est prête

    // Lire la distance
    if (interfaceVL6180x_litDistance(fd, &distance) < 0) {
        printf("Erreur: Lecture de la distance échouée.\n");
        return -1;
    }

    return distance;  // La distance lue en cm
}
