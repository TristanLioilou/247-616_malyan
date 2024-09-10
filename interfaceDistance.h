#ifndef INTERFACEMALYAN_H
#define INTERFACEMALYAN_H

#define I2C_FICHIER "/dev/i2c-1"  // Fichier de périphérique I2C (ajustez si nécessaire)
#define I2C_ADRESSE 0x29  // Adresse I2C du capteur VL6180x

// Prototypes des fonctions
int interfaceVL6180x_ecrit(int fd, uint16_t Registre, uint8_t Donnee);
int interfaceVL6180x_lit(int fd, uint16_t Registre, uint8_t *Donnee);
int interfaceVL6180x_configure(int fd);
int interfaceVL6180x_litDistance(int fd, float *Distance);
float lireDistanceCapteur(int fd);

#endif // INTERFACEMALYAN_H
