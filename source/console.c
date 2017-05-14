// Console is the generic interface to the command line.
// These functions should not need signficant modification, only
// to be called from the normal loop. Note that adding commands should
// be done in console commands.

#include <string.h>  // for NULL
#include <stdlib.h>  // for atoi and itoa (though this code implement a version of that)
#include <stdbool.h>
#include "console.h"
#include "consoleIo.h"
#include "consoleCommands.h"

#define MIN(X, Y)		(((X) < (Y)) ? (X) : (Y))
#define NOT_FOUND		-1
#define INT16_MAX_STR_LENGTH 8 // -65534: six characters plus a two NULLs
#define INT32_MAX_STR_LENGTH 16
#define NULL_CHAR            '\0'
#define CR_CHAR              '\r'
#define LF_CHAR              '\n'

// global variables
char mReceiveBuffer[CONSOLE_COMMAND_MAX_LENGTH];
uint32_t mReceivedSoFar;

// local functions
static int32_t ConsoleCommandEndline(const char receiveBuffer[], const  uint32_t filledLength);

static uint32_t ConsoleCommandMatch(const char* name, const char *buffer);
static eCommandResult_T ConsoleParamFindN(const char * buffer, const uint8_t parameterNumber, uint32_t *startLocation);
static uint32_t ConsoleResetBuffer(char receiveBuffer[], const  uint32_t filledLength, uint32_t usedSoFar);

static eCommandResult_T ConsoleUtilHexCharToInt(char charVal, uint8_t* pInt); // this might be replaceable with *pInt = atoi(str)
static eCommandResult_T ConsoleUtilsIntToHexChar(uint8_t intVal, char* pChar); // this could be replaced with itoa (intVal, str, 16);

// ConsoleCommandMatch
// Look to see if the data in the buffer matches the command name given that
// the strings are different lengths and we have parameter separators
static uint32_t ConsoleCommandMatch(const char* name, const char *buffer)
{
	uint32_t i = 0u;
	uint32_t result = 0u; // match

	if ( buffer[i] == name [i] )
	{
		result = 1u;
		i++;
	}

	while ( ( 1u == result ) &&
		( i < CONSOLE_COMMAND_MAX_COMMAND_LENGTH )  &&
		( buffer[i] != PARAMETER_SEPARATER ) &&
		( buffer[i] != LF_CHAR ) &&( buffer[i] != CR_CHAR ) &&
		( buffer[i] != (char) NULL_CHAR )
		)
	{
		if ( buffer[i] != name[i] )
		{
			result = 0u;
		}
		i++;
	}

	return result;
}

// ConsoleResetBuffer
// In an ideal world, this would just zero out the buffer. However, thre are times when the
// buffer may have data beyond what was used in the last command.
// We don't want to lose that data so we move it to the start of the command buffer and then zero
// the rest.
static uint32_t ConsoleResetBuffer(char receiveBuffer[], const uint32_t filledLength, uint32_t usedSoFar)
{
	uint32_t i = 0;

	while (usedSoFar < filledLength)
	{
		receiveBuffer[i] = receiveBuffer[usedSoFar]; // move the end to the start
		i++;
		usedSoFar++;
	}
	for ( /* nothing */ ; i < CONSOLE_COMMAND_MAX_LENGTH ; i++)
	{
		receiveBuffer[i] =  NULL_CHAR;
	}
	return (filledLength - usedSoFar);
}

// ConsoleCommandEndline
// Check to see where in the buffer stream the endline is; that is the end of the command and parameters
static int32_t ConsoleCommandEndline(const char receiveBuffer[], const  uint32_t filledLength)
{
	uint32_t i = 0;
	int32_t result = NOT_FOUND; // if no endline is found, then return -1 (NOT_FOUND)

	while ( ( CR_CHAR != receiveBuffer[i])  && (LF_CHAR != receiveBuffer[i])
			&& ( i < filledLength ) )
	{
		i++;
	}
	if ( i < filledLength )
	{
		result = i;
	}
	return result;
}

// ConsoleInit
// Initialize the console interface and all it depends on
void ConsoleInit(void)
{
	uint32_t i;

	ConsoleIoInit();
	ConsoleIoSendString("Welcome to the Consolinator, your gateway to testing code and hardware.");	
	ConsoleIoSendString(STR_ENDLINE);
	ConsoleIoSendString(CONSOLE_PROMPT);
	mReceivedSoFar = 0u;

	for ( i = 0u ; i < CONSOLE_COMMAND_MAX_LENGTH ; i++)
	{
		mReceiveBuffer[i] = NULL_CHAR;
	}

}

// ConsoleProcess
// Looks for new inputs, checks for endline, then runs the matching command.
// Call ConsoleProcess from a loop, it will handle commands as they become available
void ConsoleProcess(void)
{
	const sConsoleCommandTable_T* commandTable;
	uint32_t received;
	uint32_t cmdIndex;
	int32_t  cmdEndline;
	int32_t  found;
	eCommandResult_T result;

	ConsoleIoReceive((uint8_t*)&(mReceiveBuffer[mReceivedSoFar]), ( CONSOLE_COMMAND_MAX_LENGTH - mReceivedSoFar ), &received);
	if ( received > 0u )
	{
		mReceivedSoFar += received;
		cmdEndline = ConsoleCommandEndline(mReceiveBuffer, mReceivedSoFar);
		if ( cmdEndline >= 0 )  // have complete string, find command
		{
			commandTable = ConsoleCommandsGetTable();
			cmdIndex = 0u;
			found = NOT_FOUND;
			while ( ( NULL != commandTable[cmdIndex].name ) && ( NOT_FOUND == found ) )
			{
				if ( ConsoleCommandMatch(commandTable[cmdIndex].name, mReceiveBuffer) )
				{
					result = commandTable[cmdIndex].execute(mReceiveBuffer);
					if ( COMMAND_SUCCESS != result )
					{
						ConsoleIoSendString("Error: ");
						ConsoleIoSendString(mReceiveBuffer);

						ConsoleIoSendString("Help: ");
						ConsoleIoSendString(commandTable[cmdIndex].help);
						ConsoleIoSendString(STR_ENDLINE);

					}
					found = cmdIndex;
				}
				else
				{
					cmdIndex++;

				}
			}
			if ( ( cmdEndline != 0 ) && ( NOT_FOUND == found ) )
			{
				if (mReceivedSoFar > 2) /// shorter than that, it is probably nothing
				{
					ConsoleIoSendString("Command not found.");
					ConsoleIoSendString(STR_ENDLINE);
				}
			}
			mReceivedSoFar = ConsoleResetBuffer(mReceiveBuffer, mReceivedSoFar, cmdEndline);
			ConsoleIoSendString(CONSOLE_PROMPT);
		}
	}
}

// ConsoleParamFindN
// Find the start location of the nth parametr in the buffer where the command itself is parameter 0
static eCommandResult_T ConsoleParamFindN(const char * buffer, const uint8_t parameterNumber, uint32_t *startLocation)
{
	uint32_t bufferIndex = 0;
	uint32_t parameterIndex = 0;
	eCommandResult_T result = COMMAND_SUCCESS;


	while ( ( parameterNumber != parameterIndex ) && ( bufferIndex < CONSOLE_COMMAND_MAX_LENGTH ) )
	{
		if ( PARAMETER_SEPARATER == buffer[bufferIndex] )
		{
			parameterIndex++;
		}
		bufferIndex++;
	}
	if  ( CONSOLE_COMMAND_MAX_LENGTH == bufferIndex )
	{
		result = COMMAND_PARAMETER_ERROR;
	}
	else
	{
		*startLocation = bufferIndex;
	}
	return result;
}

// ConsoleReceiveParamInt16
// Identify and obtain a parameter of type int16_t, sent in in decimal, possibly with a negative sign.
// Note that this uses atoi, a somewhat costly function. You may want to replace it, see ConsoleReceiveParamHexUint16
// for some ideas on how to do that.
eCommandResult_T ConsoleReceiveParamInt16(const char * buffer, const uint8_t parameterNumber, int16_t* parameterInt)
{
	uint32_t startIndex = 0;
	uint32_t i;
	eCommandResult_T result;
	char charVal;
	char str[INT16_MAX_STR_LENGTH];

	result = ConsoleParamFindN(buffer, parameterNumber, &startIndex);

	i = 0;
	charVal = buffer[startIndex + i];
	while ( ( LF_CHAR != charVal ) && ( CR_CHAR != charVal )
			&& ( PARAMETER_SEPARATER != charVal )
		&& ( i < INT16_MAX_STR_LENGTH ) )
	{
		str[i] = charVal;					// copy the relevant part
		i++;
		charVal = buffer[startIndex + i];
	}
	if ( i == INT16_MAX_STR_LENGTH)
	{
		result = COMMAND_PARAMETER_ERROR;
	}
	if ( COMMAND_SUCCESS == result )
	{
		str[i] = NULL_CHAR;
		*parameterInt = atoi(str);
	}
	return result;
}

// ConsoleReceiveParamHexUint16
// Identify and obtain a parameter of type uint16, sent in as hex. This parses the number and does not use
// a library function to do it.
eCommandResult_T ConsoleReceiveParamHexUint16(const char * buffer, const uint8_t parameterNumber, uint16_t* parameterUint16)
{
	uint32_t startIndex = 0;
	uint16_t value = 0;
	uint32_t i;
	eCommandResult_T result;
	uint8_t tmpUint8;

	result = ConsoleParamFindN(buffer, parameterNumber, &startIndex);
	if ( COMMAND_SUCCESS == result )
	{
		// bufferIndex points to start of integer
		// next separator or newline or NULL indicates end of parameter
		for ( i = 0u ; i < 4u ; i ++)   // U16 must be less than 4 hex digits: 0xFFFF
		{
			if ( COMMAND_SUCCESS == result )
			{
				result = ConsoleUtilHexCharToInt(buffer[startIndex + i], &tmpUint8);
			}
			if ( COMMAND_SUCCESS == result )
			{
				value = (value << 4u);
				value += tmpUint8;
			}
		}
		if  ( COMMAND_PARAMETER_END == result )
		{
			result = COMMAND_SUCCESS;
		}
		*parameterUint16 = value;
	}
	return result;
}

// ConsoleSendParamHexUint16
// Send a parameter of type uint16 as hex.
// This does not use a library function to do it (though you could
// do itoa (parameterUint16, out, 16);  instead of building it up
eCommandResult_T ConsoleSendParamHexUint16(uint16_t parameterUint16)
{
	uint32_t i;
	char out[4u + 1u];  // U16 must be less than 4 hex digits: 0xFFFF, end buffer with a NULL
	eCommandResult_T result = COMMAND_SUCCESS;
	uint8_t tmpUint8;

	for ( i = 0u ; i < 4u ; i ++)   // U16 must be less than 4 hex digits: 0xFFFF
	{
		if ( COMMAND_SUCCESS == result )
		{
			tmpUint8 = ( parameterUint16 >> (12u - (i*4u)) & 0xF);
			result = ConsoleUtilsIntToHexChar(tmpUint8, &(out[i]));
		}
	}
	out[i] = NULL_CHAR;
	ConsoleIoSendString(out);

	return COMMAND_SUCCESS;
}

eCommandResult_T ConsoleSendParamHexUint8(uint8_t parameterUint8)
{
	uint32_t i;
	char out[2u + 1u];  // U8 must be less than 4 hex digits: 0xFFFF, end buffer with a NULL
	eCommandResult_T result = COMMAND_SUCCESS;

	i = 0u;
	result = ConsoleUtilsIntToHexChar( (parameterUint8 >> 4u) & 0xF, &(out[i]));
	i++;
	if ( COMMAND_SUCCESS == result )
	{
		result = ConsoleUtilsIntToHexChar( parameterUint8 & 0xF, &(out[i]));
		i++;
	}
	out[i] = NULL_CHAR;
	ConsoleIoSendString(out);

	return COMMAND_SUCCESS;
}

// The C library itoa is sometimes a complicated function and the library costs aren't worth it
// so this is implements the parts of the function needed for console.
static void __itoa(int in, char* outBuffer, int radix)
{
	bool isNegative = false;
	int tmpIn;
	int stringLen = 1u; // it will be at least as long as the NULL character

	if (in < 0) {
		isNegative = true;
		in = -in;
		stringLen++;
	}

	tmpIn = in;
	while ((int)tmpIn/radix != 0) {
		tmpIn = (int)tmpIn/radix;
		stringLen++;
	}
    
    // Now fill it in backwards, starting with the NULL at the end
    *(outBuffer + stringLen) = NULL_CHAR;
    stringLen--;

	tmpIn = in;
	do {
		*(outBuffer+stringLen) = (tmpIn%radix)+'0';
		tmpIn = (int) tmpIn / radix;
	} while(stringLen--);

	if (isNegative) {
		*(outBuffer) = '-';
	}
}


// ConsoleSendParamInt16
// Send a parameter of type int16 using the (unsafe) C library function
// __itoa to translate from integer to string.
eCommandResult_T ConsoleSendParamInt16(int16_t parameterInt)
{
	char out[INT16_MAX_STR_LENGTH];
//	memset(out, 0, INT16_MAX_STR_LENGTH);

	__itoa (parameterInt, out, 10);
	ConsoleIoSendString(out);

	return COMMAND_SUCCESS;
}

// ConsoleSendParamInt32
// Send a parameter of type int16 using the (unsafe) C library function
// __itoa to translate from integer to string.
eCommandResult_T ConsoleSendParamInt32(int32_t parameterInt)
{
	char out[INT32_MAX_STR_LENGTH];
	memset(out, 0, sizeof(out));

	__itoa (parameterInt, out, 10);
	ConsoleIoSendString(out);

	return COMMAND_SUCCESS;
}
// ConsoleUtilHexCharToInt
// Converts a single hex character (0-9,A-F) to an integer (0-15)
static eCommandResult_T ConsoleUtilHexCharToInt(char charVal, uint8_t* pInt)
{
    eCommandResult_T result = COMMAND_SUCCESS;

    if ( ( '0' <= charVal ) && ( charVal <= '9' ) )
    {
        *pInt = charVal - '0';
    }
    else if ( ( 'A' <= charVal ) && ( charVal <= 'F' ) )
    {
        *pInt = 10u + charVal - 'A';
    }
    else if( ( 'a' <= charVal ) && ( charVal <= 'f' ) )
    {
        *pInt = 10u + charVal - 'a';
    }
	else if ( ( LF_CHAR != charVal ) || ( CR_CHAR != charVal )
			|| ( PARAMETER_SEPARATER == charVal ) )
	{
		result = COMMAND_PARAMETER_END;

	}
    else
    {
        result = COMMAND_PARAMETER_ERROR;
    }

    return result;
}
// ConsoleUtilsIntToHexChar
// Converts an integer nibble (0-15) to a hex character (0-9,A-F)
static eCommandResult_T ConsoleUtilsIntToHexChar(uint8_t intVal, char* pChar)
{
    eCommandResult_T result = COMMAND_SUCCESS;

    if ( intVal <= 9u )
    {
        *pChar = intVal + '0';
    }
    else if ( ( 10u <= intVal ) && ( intVal <= 15u ) )
    {
        *pChar = intVal - 10u + 'A';
    }
    else
    {
        result = COMMAND_PARAMETER_ERROR;
    }

    return result;
}