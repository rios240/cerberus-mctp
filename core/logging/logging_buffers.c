#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "platform_io.h"
#include "logging_buffers.h"


void logBuffer(uint8_t* buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        platform_printf("%02X ", buffer[i]);
    }
    platform_printf("\n");
}