// This is a wrapper between the actual in and output and the console code
// In an embedded system, this might interace to a UART driver, an LED, or 
// a flash logging subsystem.
// For the PC version, this is simply standard IO (printf)

#include <stdio.h>
#include "debug.h"

void DebugString(char * str)
{
	printf("%s", str);
	printf(STR_ENDLINE);
}
