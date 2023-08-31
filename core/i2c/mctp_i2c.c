#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "mctp_i2c.h"
#include "cmd_interface/cmd_channel.h"

#define PORT 5600
#define SERVER_ADDRESS "127.0.0.1"

int receive_mctp_message(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return MCTP_I2C_INIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &(server.sin_addr)) <= 0) {
        close(sock);
        return MCTP_I2C_INET_PTON_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return MCTP_I2C_CONNECT_FAILURE;
    }
    
    

    ssize_t bytes_received = recv(sock, packet->data, sizeof (packet->data), 0);
    if (bytes_received < 0) {
        close(sock);
        return MCTP_I2C_RECV_FAILURE;
    }

    close(sock);

    return 0;
}

int send_mctp_packet(struct cmd_channel *channel, struct cmd_packet *packet) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return MCTP_I2C_INIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &(server.sin_addr)) <= 0) {
        close(sock);
        return MCTP_I2C_INET_PTON_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sock);
        return MCTP_I2C_CONNECT_FAILURE;
    }


    if (send(sock, packet->data, sizeof (packet->data), 0) < 0) {
        close(sock);
        return MCTP_I2C_SEND_FAILURE;
    }

    
    close(sock);

    return 0;
}

