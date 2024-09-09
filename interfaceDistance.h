#ifndef INTERfACEDISTANCE_H
#define INTERfACEDISTANCE_H

//DESCRIPTION: 
//HISTORIQUE:


//DEFINITIONS REQUISES PAR LE PROGRAMME:

//Dependances materielles:

//Dependances logicielles:
#include <stdint.h>
#include <stdio.h>

//INFORMATION PUBLIQUE:
//Definitions publiques:
#define I2C_FICHIER "/dev/i2c-1"
#define I2C_ADRESSE 0x29


//Fonctions publiques:
int interfaceVL6180x_ecrit(int fd, uint16_t Registre, uint8_t Donnee);
int interfaceVL6180x_lit(int fd, uint16_t Registre, uint8_t *Donnee);
int interfaceVL6180x_configure(int fd);
int interfaceVL6180x_litDistance(int fd, float *Distance);
int FonctionDistance(void);

//Variables publiques:
//int fdPortI2C;  // port I2C
//float distance;
//int DistanceLue;

#endif