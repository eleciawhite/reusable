// Test file for reusable code.


#include "debug.h"
#include "console.h"

int main () 
{
	DebugString("Welcome to the Consolinator, your gateway to testing code and hardware.");
	ConsoleInit();

	while(1) 
	{
		ConsoleProcess();
	}	

}