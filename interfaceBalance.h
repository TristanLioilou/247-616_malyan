#ifndef INTERFACEBALANCE_H
#define INTERFACEBALANCE_H

//MODULE: interfaceBalance
//DESCRIPTION: Module qui permet de recevoir des messages de la balance

//HISTORIQUE:
// 2024-11-25, Nicolas Proteau

//Fonctions publiques:

int configure_port(const char *port_name);
int read_response(int fd_balance, char *buffer, size_t buffer_size);
int detect_port();
int reconnect(int *fd_balance);
void send_command(int fd_balance, const char *command);

#endif