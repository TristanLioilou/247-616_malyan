#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdint.h>  
#include <stdlib.h> 

#include "listener.h"

extern char Fonction;
extern char Couleur;
extern char PosVéhicule;

const char *can_interface = "can0";  // Interface CAN à utiliser (modifier si nécessaire)
void send_can_message(int can_socket, uint32_t id, const char *data, int length) {
    struct can_frame frame;
    frame.can_id = id;
    frame.can_dlc = length;
    memcpy(frame.data, data, length);

    if (write(can_socket, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        perror("Erreur d'envoi du message CAN");
    } else {
        //printf("Message CAN ID 0x%x envoyé\n", id);
    }
}

void respond_to_address_with_03_in_second_octet(int can_socket, struct can_frame *frame, char *center_content, char *second_content) {
    // Vérifier si le 2ème octet de l'ID est 0x101 (ou autre condition selon le besoin)
    if (frame->can_id == 0x101)// 101
    {
// Construction de la première trame avec un début "$" et une fin "\n"
char *decimal_point = strchr(center_content, '.');
size_t response_size = strlen(center_content) + 3;  // 1 pour "$", 1 pour "\n", et taille du centre
char response[response_size + 1];  // +1 pour le caractère '\0' de fin de chaîne

// Initialiser à zéro pour éviter des valeurs résiduelles
memset(response, 0, sizeof(response));

// Si le centre ne contient pas de point décimal, la chaîne représente uniquement un nombre entier
if (decimal_point == NULL) {
    int integer_value = atoi(center_content);  // Conversion de la chaîne en entier
    response[1] = (unsigned char)integer_value; // Placer la partie entière dans le tableau
    response[2] = 0x00; // Pas de décimale
} else {
    // Séparer la partie entière et la partie décimale
    char integer_part[10];
    char decimal_part[10];

    // Extraire la partie entière (avant le point)
    int integer_length = decimal_point - center_content;
    strncpy(integer_part, center_content, integer_length);
    integer_part[integer_length] = '\0'; // Terminer la chaîne

    // Extraire la partie décimale (après le point)
    strcpy(decimal_part, decimal_point + 1);

    // Convertir la partie entière en entier
    int integer_value = 0;
    for (int i = 0; i < integer_length; i++) {
        integer_value = integer_value * 10 + (integer_part[i] - '0');  // Convertir caractère par caractère
    }

    // Convertir la partie décimale en entier
    int decimal_value = 0;
    int decimal_length = strlen(decimal_part);
    for (int i = 0; i < decimal_length && i < 2; i++) {  // Limiter à 2 chiffres après la virgule
        decimal_value = decimal_value * 10 + (decimal_part[i] - '0');
    }

    // Placer la partie entière dans le premier octet
    response[1] = (unsigned char)integer_value;

    // Placer la partie décimale dans le second octet (max 99 centièmes)
    if (decimal_value >= 100) {
        decimal_value = 99;  // Limite à deux chiffres (centièmes)
    }
    response[2] = (unsigned char)decimal_value;
}

// Placer le '$' au début et le '\n' à la fin
response[0] = '$';
response[3] = '\n';  // Utiliser response_size - 1 pour le '\n'

// Envoyer la première trame
send_can_message(can_socket, 0x311, response, 4);

// Afficher les valeurs envoyées
printf("Transmission: 0x%02X 0x%02X\n", response[1], response[2]);


size_t second_response_size = strlen(second_content) + 2;  // Taille totale nécessaire (1 pour '$', 1 pour '\n')
char second_response[second_response_size + 1];  // +1 pour l'espace de fin de chaîne '\0'
// Initialiser le tableau à 0 pour éviter les valeurs résiduelles
memset(second_response, 0, sizeof(second_response));
// Premier caractère est '$'
second_response[0] = '$';
// Copier tout le contenu de 'second_content' après '$'
memcpy(second_response + 1, second_content, strlen(second_content));
// Ajouter '\n' à la fin de la chaîne
second_response[second_response_size - 1] = '\n';  // Remarquer que c'est second_response_size - 1
// Ajouter le caractère de fin de chaîne '\0'
second_response[second_response_size] = '\0';
// Afficher le message
printf("Message: %s\n", second_response);
// Envoyer la trame
send_can_message(can_socket, 0x301, second_response, second_response_size);  // Envoyer la trame
    }
 if (((frame->can_id & 0x0F0) == 0x000)||((frame->can_id & 0x0F0) == 0x030))// 101
 {
    if((frame->can_id & 0xF00) == 0x100)
    {
        printf("Message reçu : 0x%02X ", frame->data[1]);  // Affichage des données de la trame
        Fonction = frame->data[1];
    }
    if((frame->can_id & 0xF00) == 0x200)
    {
        printf("Message reçu : 0x%02X ", frame->data[2]);  // Affichage des données de la trame
        Couleur = frame->data[2];
    }
    if((frame->can_id & 0xF00) == 0x400)
    {
        printf("Message reçu 1: 0x%02X ", frame->data[2]);
        printf("Message reçu 2: 0x%02X ", frame->data[3]);  // Affichage des données de la trame
        if(frame->data[2] == frame->data[3])
        {
            PosVéhicule = frame->data[3];
        }  
    }
 }
}

// Fonction pour lire et répondre aux messages CAN
void read_and_respond_to_can(int can_socket, char *center_content, char *second_content) {
    struct can_frame frame;
    int nbytes;

    while (1) {
        nbytes = read(can_socket, &frame, sizeof(struct can_frame));
        if (nbytes < 1) {
            perror("Erreur de lecture du message CAN");
            continue;
        }
if (center_content != NULL) {
    printf("Message dans Can: %s\n", center_content);
} else {
    printf("Le message est NULL.\n");
}
        // Répondre avec les données passées
        respond_to_address_with_03_in_second_octet(can_socket, &frame, center_content, second_content);
    }
}

