#ifndef SOCKET_CONNECT_H
#define SOCKET_CONNECT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Initialize the socket connection
int initialize_socket_connection();

// Receive a PLDM message over the socket
int socket_receive_pldm_message(uint8_t* data, size_t max_data_length);

// Close the socket connection
int close_socket_connection();

#ifdef __cplusplus
}
#endif

#endif  // SOCKET_CONNECT_H