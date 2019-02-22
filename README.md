# Reusable
Reusable code for Logical Elegance, Inc. clients 

Note that this is open source, so anyone can reuse it, but I'm only supporting it for myself.

Console.c is a command parser. Call the init function during init and then call 
the process function on every pass through a loop. 

The command table and commands are implemented in consoleCommands.c.

The interface is in consoleIo.c which is currently an sdio interface to a 
Windows command console, but it is likely to be a UART in an embedded system.

There is also a debug interface which will likely get expanded as time goes on.
