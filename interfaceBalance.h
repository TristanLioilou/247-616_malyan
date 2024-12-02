#ifndef PILOTE_BALANCE_H
#define PILOTE_BALANCE_H

#include <stddef.h> // Pour size_t

// Définition des constantes
#define BAUD_RATE B9600           // Baud rate de la communication série
#define MAX_RECONNECT_ATTEMPTS 5  // Nombre maximum de tentatives de connexion (USB -> ttyO0 -> ttyO1)
#define READ_TIMEOUT 5            // Timeout en secondes pour la lecture
#define BUFFER_SIZE 256           // Taille du buffer pour la lecture des réponses

// Liste des ports à tester
extern const char *devices[];

// Déclarations des fonctions

// Fonction pour configurer le port série
int configure_port(const char *port_name);

// Fonction pour lire une réponse depuis le port série
int read_response(int fd_balance, char *buffer, size_t buffer_size);

// Fonction pour valider une réponse (vérifier un préfixe)
int validate_response(const char *response, const char *expected_prefix);

// Fonction pour détecter un port série valide
int detect_port();

// Fonction pour tenter une reconnexion sur un autre port
int reconnect(int *fd_balance);

// Fonction pour envoyer une commande à la balance
void send_command(int fd_balance, const char *command);

#endif // PILOTE_BALANCE_H
