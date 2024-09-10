#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "main.h"
#include "interfaceBalance.h"

#define BAUD_RATE B9600           // Baud rate de la communication série
#define MAX_RECONNECT_ATTEMPTS 5  // Nombre maximum de tentatives de connexion (USB -> ttyO0 -> ttyO1)
#define READ_TIMEOUT 5            // Timeout en secondes pour la lecture
#define BUFFER_SIZE 256

// Liste des ports à tester
const char *devices[] = {
    "/dev/ttyUSB0",
    "/dev/ttyUSB1",
    "/dev/ttyS0",
    "/dev/ttyS1"
};

// Fonction pour configurer le port série
int configure_port(const char *port_name) {
    int fd_balance = open(port_name, O_RDWR | O_NOCTTY);
    if (fd_balance == -1) {
        perror("Erreur d'ouverture du port série");
        return -1;
    }

    struct termios options;
    if (tcgetattr(fd_balance, &options) < 0) {
        perror("Erreur de récupération des attributs du port");
        close(fd_balance);
        return -1;
    }

    cfsetispeed(&options, BAUD_RATE);
    cfsetospeed(&options, BAUD_RATE);

    options.c_cflag |= (CLOCAL | CREAD); // Activer la lecture et ignorer les signaux
    options.c_cflag &= ~PARENB; // Pas de parité
    options.c_cflag &= ~CSTOPB; // 1 bit de stop
    options.c_cflag &= ~CSIZE;  // Masquer les bits de taille
    options.c_cflag |= CS8;     // 8 bits de données

    options.c_cc[VMIN] = 1;     // Minimum 1 caractère à lire
    options.c_cc[VTIME] = 20;   // Timeout de 2 secondes
    fcntl(fd_balance, F_SETFL, O_NONBLOCK);

    tcflush(fd_balance, TCIFLUSH);
    if (tcsetattr(fd_balance, TCSANOW, &options) < 0) {
        perror("Erreur d'application de la configuration");
        close(fd_balance);
        return -1;
    }

    return fd_balance;
}

int read_response(int fd_balance, char *buffer, size_t buffer_size) {
    memset(buffer, 0, buffer_size);
    size_t total_bytes = 0;
    int retries = 5; // Nombre de tentatives
    int bytes_read;

    fcntl(fd_balance, F_SETFL, O_NONBLOCK);  // Passer le port en mode non-bloquant

    while (total_bytes < buffer_size - 1 && retries > 0) {
        bytes_read = read(fd_balance, buffer + total_bytes, buffer_size - total_bytes - 1);

        if (bytes_read > 0) {
            total_bytes += bytes_read;
            if (buffer[total_bytes - 1] == '\n') {
                break; // Si une ligne complète est lue, on sort
            }
        } else if (bytes_read == 0) {
            usleep(100000); // Attendre 100 ms avant de réessayer
            retries--;
        } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Erreur de lecture");
            return -1;
        }
    }

    buffer[total_bytes] = '\0'; // Terminer la chaîne par un null byte
    return total_bytes > 0 ? 0 : -1;  // Retourner 0 si la lecture a réussi
}

int validate_response(const char *response, const char *expected_prefix) {
    if (strncmp(response, expected_prefix, strlen(expected_prefix)) == 0) {
        return 1; // Réponse valide
    }
    return 0; // Réponse invalide
}

// Fonction pour détecter le port de la balance
int detect_port() {
    char test_command[] = "T<cr><lf>"; // Commande de tare
    char response[BUFFER_SIZE];

    for (int i = 0; i < 4; i++) {
        int fd_balance = open(devices[i], O_RDWR | O_NOCTTY);
        if (fd_balance == -1) {
            printf("Port %s non trouvé.\n", devices[i]);
            continue;
        }
        close(fd_balance);

        int fd_balance_configured = configure_port(devices[i]);
        if (fd_balance_configured != -1) {
            send_command(fd_balance_configured, test_command);
            sleep(1);
            if (read_response(fd_balance_configured, response, sizeof(response)) == 0) {
                close(fd_balance_configured);
                return i; // Retourner l'indice du port trouvé
            }
            close(fd_balance_configured);
            
        } else {
            printf("Impossible d'ouvrir %s.\n", devices[i]);
        }
    }
    return -1;
}

// Fonction pour tenter une reconnexion
int reconnect(int *fd_balance) {
    for (int attempt = 0; attempt < MAX_RECONNECT_ATTEMPTS; attempt++) {
        int port_index = detect_port();
        if (port_index != -1) {
            const char *port_name = devices[port_index];
            *fd_balance = configure_port(port_name);
            if (*fd_balance != -1) {
                printf("Reconnecté à la balance sur %s\n", port_name);
                return 0;
            }
        }
        sleep(1);
    }
    fprintf(stderr, "Échec de la reconnexion après %d tentatives.\n", MAX_RECONNECT_ATTEMPTS);
    return -1;
}

// Fonction pour envoyer une commande à la balance
void send_command(int fd_balance, const char *command) {
    tcflush(fd_balance, TCIFLUSH);
    if (write(fd_balance, command, strlen(command)) == -1) {
        perror("Erreur lors de l'envoi de la commande");
    }
    tcdrain(fd_balance); // Attendre que la commande soit complètement envoyée
    usleep(300000); // Attendre 300 ms pour permettre à la balance de répondre
}
