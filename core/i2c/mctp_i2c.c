#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "mctp_i2c.h"

#define PORT 5600
#define SERVER_ADDRESS "127.0.0.1"

int socket_receive_pldm_message(uint8_t* buffer, size_t buffer_size) {
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
    
    

    ssize_t bytes_received = recv(sock, buffer, buffer_size, 0);
    if (bytes_received < 0) {
        close(sock);
        return MCTP_I2C_RECV_FAILURE;
    }

    close(sock);

    return bytes_received;
}

int socket_send_mctp_packet(uint8_t* buffer, size_t buffer_size) {

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


    if (send(sock, buffer, buffer_size, 0) < 0) {
        close(sock);
        return MCTP_I2C_SEND_FAILURE;
    }

    
    close(sock);

    return 0;
}

