#ifndef LISTENER_H
#define LISTENER_H

#include <linux/can.h>
#include <stdint.h>  // Ajout pour les types uint32_t, etc.

// Interface CAN à utiliser
extern const char *can_interface;

// Fonction pour initialiser le socket CAN
int setup_can_socket();

// Fonction pour envoyer un message CAN
void send_can_message(int can_socket, uint32_t id, const char *data, int length);

// Fonction pour répondre avec la trame "$BJ\n" si le 2ème octet est 0x03
void respond_to_address_with_03_in_second_octet(int can_socket, struct can_frame *frame, char *center_content, char *second_content);

// Fonction pour lire les messages CAN et répondre si nécessaire
void read_and_respond_to_can(int can_socket, char *center_content, char *second_content);

#endif