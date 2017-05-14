// Console is the generic interface to the command line. 
// These functions should not need signficant modification, only 
// to be called from the normal loop. Note that adding commands should 
// be done in console commands. 
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

// User configuration
#define CONSOLE_PROMPT			("> ")
#define PARAMETER_SEPARATER		(' ')
#define STR_ENDLINE "\r\n"

// Salled from higher up areas of the code (main)
void ConsoleInit(void);
void ConsoleProcess(void); // call this in a loop

// called from lower down areas of the code (consoleCommands)
typedef enum { 
	COMMAND_SUCCESS = 0u, 
	COMMAND_PARAMETER_ERROR = 0x10u, 
	COMMAND_PARAMETER_END   = 0x11u,
	COMMAND_ERROR =0xFFu 
} eCommandResult_T;

// The in and output of the int16 parameter use C standard library functions
// atoi and itoa. These are nice functions, usually a lot smaller than scanf and printf
// but they can be memory hogs in their flexibility. 
// The HexUint16 functions implement the parsing themselves, eschewing atoi and itoa.
eCommandResult_T ConsoleReceiveParamInt16(const char * buffer, const uint8_t parameterNumber, int16_t* parameterInt16);
eCommandResult_T ConsoleSendParamInt16(int16_t parameterInt);
eCommandResult_T ConsoleSendParamInt32(int32_t parameterInt);
eCommandResult_T ConsoleReceiveParamHexUint16(const char * buffer, const uint8_t parameterNumber, uint16_t* parameterUint16);
eCommandResult_T ConsoleSendParamHexUint16(uint16_t parameterUint16);
eCommandResult_T ConsoleSendParamHexUint8(uint8_t parameterUint8);

#endif // CONSOLE_H
