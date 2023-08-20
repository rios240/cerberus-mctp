#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "platform_io.h"
#include "logging_mctp_buffers.h"


void logBufferToFile(const uint8_t *tx_packet, const uint8_t *msg_buf, size_t length1, size_t length2) {

    size_t maxLength = length1 > length2 ? length1 : length2;

    platform_printf("Byte       tx_packet     msg_buf" NEWLINE);
    for (size_t i = 0; i < maxLength; ++i) {
        uint8_t tx_byte = (i < length1) ? tx_packet[i] : 0;
        uint8_t msg_byte = (i < length2) ? msg_buf[i] : 0;
        platform_printf("Byte %3zu:    0x%02X        0x%02X" NEWLINE, i, tx_byte, msg_byte);
    }
}