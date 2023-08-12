#ifndef SOCKET_CONNECT_H
#define SOCKET_CONNECT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



// Receive a PLDM message over the socket
int socket_receive_pldm_message(uint8_t* buffer, size_t buffer_size);



#ifdef __cplusplus
}
#endif

#endif  // SOCKET_CONNECT_H