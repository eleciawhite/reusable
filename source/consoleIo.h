// Console IO is a wrapper between the actual in and output and the console code

#ifndef CONSOLE_IO_H
#define CONSOLE_IO_H

#include <stdint.h>

typedef enum {CONSOLE_SUCCESS = 0u, CONSOLE_ERROR = 1u } eConsoleError;

eConsoleError ConsoleIoInit(void);

eConsoleError ConsoleIoReceive(uint8_t *buffer, const uint32_t bufferLength, uint32_t *readLength);
eConsoleError ConsoleIoSend(const uint8_t *buffer, const uint32_t bufferLength, uint32_t *sentLength);
eConsoleError ConsoleIoSendString(const char *buffer); // must be null terminated

#endif // CONSOLE_IO_H
