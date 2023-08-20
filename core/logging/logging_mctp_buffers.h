#ifndef LOGGING_MCTP_BUFFERS_H_
#define LOGGING_MCTP_BUFFERS_H_

#include <stdint.h>
#include <stdio.h>

void logBufferToFile(const uint8_t *tx_packet, const uint8_t *msg_buf, size_t length1, size_t length2);



#endif // LOGGING_MCTP_BUFFERS_H_