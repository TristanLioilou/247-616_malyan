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

const char *can_interface = "can0"; // Interface CAN à utiliser (modifier si nécessaire)
void send_can_message(int can_socket, uint32_t id, const char *data, int length)
{
    struct can_frame frame;
    frame.can_id = id;
    frame.can_dlc = length;
    memcpy(frame.data, data, length);

    if (write(can_socket, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
    {
        perror("Erreur d'envoi du message CAN");
    }
    else
    {
        // printf("Message CAN ID 0x%x envoyé\n", id);
    }
}

void respond_to_address_with_03_in_second_octet(int can_socket, struct can_frame *frame, char center_content[255], char second_content)
{
    // Vérifier si le 2ème octet de l'ID est 0x101 (ou autre condition selon le besoin)
    if (frame->can_id == 0x101) // 101
    {
        size_t main_response_size = 3;
        char reponse[main_response_size];

        memset(reponse, 0, sizeof(reponse));
        // Premier caractère est '$'
        reponse[0] = '$';

        reponse[1] = Fonction;
        // Copier tout le contenu de 'second_content' après '$'
        // Ajouter '\n' à la fin de la chaîne
        reponse[2] = '\n'; // Remarquer que c'est second_response_size - 1
        // Ajouter le caractère de fin de chaîne '\0'
        // second_response[3] = '\0';
        // Afficher le message
        printf("Réponse Fonction %s\n", reponse);
        // Envoyer la trame
        send_can_message(can_socket, 0x311, reponse, main_response_size);
        // Construction de la première trame avec un début "$" et une fin "\n"
        size_t response_size = 4; // 1 pour "$", 1 pour "\n", et taille du centre
        char response[response_size];                  // +1 pour le caractère '\0' de fin de chaîne

        // Initialiser à zéro pour éviter des valeurs résiduelles
        memset(response, 0, sizeof(response));
        printf("Réponse Poids: %c%c\n", center_content[0], center_content[1]);
        response[1] = 0X00;
        response[2] = 0X00;
        if (center_content[0] != 0 && center_content[1] != 0)
        {
            
            char *position = strchr(center_content, '.'); // Recherche du premier '.'
            int index;
            if (position != NULL)
            {
                // Calculer la position en index
                index = position - center_content;
               // printf("Le premier '.' est à la position %d.\n", index);

                if (index == 2)
                {
                    response[1] = ((center_content[0] * 10) + (center_content[1])-16);
                    response[2] = ((center_content[3] * 10) + (center_content[4])-16);
                }
                else if (index == 3)
                {
                    response[1] = ((center_content[0] * 100) + (center_content[1] * 10) + (center_content[2])-116);
                    response[2] = ((center_content[4] * 10) + (center_content[5])-16);
                }
                else
                {
                    printf("Donner invalide");
                }
            }
            else
            {
                printf("Donner invalide");
            }
            // Placer le '$' au début et le '\n' à la fin
            response[0] = '$';
            response[3] = '\n'; // Utiliser response_size - 1 pour le '\n'

            send_can_message(can_socket, 0x311, response, 4);

            // Envoyer la première trame

            // Afficher les valeurs envoyées
            printf("Transmission: 0x%02X 0x%02X\n", response[1], response[2]);
        }
        if (second_content == 'M')
        {
            size_t second_response_size = 4;                // Taille totale nécessaire (1 pour '$', 1 pour '\n')
            char second_response[second_response_size]; 
            // Initialiser le tableau à 0 pour éviter les valeurs résiduelles
            memset(second_response, 0, sizeof(second_response));
            // Premier caractère est '$'
            second_response[0] = '$';
            // Copier tout le contenu de 'second_content' après '$'
            second_response[1] = 'T';
            second_response[2] = 'G';
            // Ajouter '\n' à la fin de la chaîne
            second_response[3] = '\n'; // Remarquer que c'est second_response_size - 1
            // Ajouter le caractère de fin de chaîne '\0'
            // second_response[3] = '\0';
            // Afficher le message
            printf("Message: %s\n", second_response);
            // Envoyer la trame
            send_can_message(can_socket, 0x341, second_response, second_response_size); // Envoyer la trame
        }
    }
    if (((frame->can_id & 0x0F0) == 0x000) || ((frame->can_id & 0x0F0) == 0x030)) // 101
    {
        if ((frame->can_id & 0xF00) == 0x100)
        {
            printf("Message reçu1 : %c\n", frame->data[1]); // Affichage des données de la trame
            Fonction = frame->data[1];
        }
        if ((frame->can_id & 0xF00) == 0x200)
        {
            printf("Message reçu2 : %c\n", frame->data[2]); // Affichage des données de la trame
            Couleur = frame->data[2];
        }
        if ((frame->can_id & 0xF00) == 0x400)
        {
            printf("Message reçu3 1: %c\n", frame->data[2]);
            printf("Message reçu4 2: %c\n", frame->data[3]); // Affichage des données de la trame
            if (frame->data[2] == frame->data[3])
            {
                PosVéhicule = frame->data[3];
            }
        }
    }
}

// Fonction pour lire et répondre aux messages CAN
void read_and_respond_to_can(int can_socket, char center_content[255], char second_content)
{
    struct can_frame frame;
    int nbytes;

    // Lecture d'une trame CAN
    nbytes = read(can_socket, &frame, sizeof(struct can_frame));
    if (nbytes < 1)
    {
        perror("Erreur de lecture du message CAN");
        return; // Quitter la fonction si la lecture échoue
    }

    // Affichage du message CAN
    if (center_content != NULL)
    {
        printf("Message dans Can: %s\n", center_content);
    }
    else
    {
        printf("Le message est NULL.\n");
    }

    // Répondre avec les données passées
    respond_to_address_with_03_in_second_octet(can_socket, &frame, center_content, second_content);
}
