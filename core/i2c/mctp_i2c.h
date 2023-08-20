#ifndef SOCKET_CONNECT_H
#define SOCKET_CONNECT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MCTP_I2C_INIT_FAILURE                   -1
#define MCTP_I2C_INET_PTON_FAILURE              -2
#define MCTP_I2C_CONNECT_FAILURE                -3
#define MCTP_I2C_RECV_FAILURE                   -4
#define MCTP_I2C_SEND_FAILURE                   -5

// Receive a PLDM message over the socket
int socket_receive_pldm_message(uint8_t* buffer, size_t buffer_size);

// Send a MCTP message over the socket
int socket_send_mctp_packet(uint8_t* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif  // SOCKET_CONNECT_H