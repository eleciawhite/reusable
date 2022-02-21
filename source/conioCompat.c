

//only necessary for non WIN32 systems
#ifndef _WIN32

/*
Sources:
https://stackoverflow.com/a/448982/910094
https://stackoverflow.com/a/58454101/910094
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>


static void initConioCompat() {
    static int needsInit = 1;
    if (needsInit) {
        needsInit = 0;
        struct termios t;
        tcgetattr(0, &t);
        t.c_lflag &= ~ICANON;
        tcsetattr(0, TCSANOW, &t);

        fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    }
}

int _kbhit()
{
    initConioCompat();
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

int _getch()
{
    initConioCompat();
    char c;
    ssize_t count = read (0, &c, 1);
    if (count > 0)
        return c;
    else
        return EOF;
}


#endif
