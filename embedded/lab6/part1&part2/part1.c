#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define video_BYTES 8       // num chars to read from /dev/video
#define BOUND_BYTES 4

int screen_x, screen_y;

void init_command(char []);
void clear_command(char []);

int main(int argc, char *argv[])
{
    int video_FD, i, j, carp;
    char buffer[video_BYTES], xbuf[BOUND_BYTES], ybuf[BOUND_BYTES], hbuf[BOUND_BYTES+1];
    char command[64];
    int x, y, color;

    if ((video_FD = open("/dev/video", O_RDWR)) == -1) {
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }

    // get screen bounds
    carp = read(video_FD, buffer, video_BYTES);
    for (i = 0; buffer[i] != ' '; i++)
        xbuf[i] = buffer[i];
    xbuf[i++] = '\0';
    for (j = 0; j < BOUND_BYTES-1; i++, j++)
        ybuf[j] = buffer[i];
    ybuf[j] = '\0';
    screen_x = atoi(xbuf);
    screen_y = atoi(ybuf);
    color = 0;
    for (x = 0; x < screen_x; x++) {
        for (y = 0; y < screen_y; y++) {
            if (color >= 255) {
                color = 0;
            }
            init_command(command);
            sprintf(xbuf, "%d", x);
            sprintf(ybuf, "%d", y);
            strcat(command, xbuf);
            strcat(command, ",");
            strcat(command, ybuf);
            strcat(command, " ");
            sprintf(hbuf, "0x%X", color);
            strcat(command, hbuf);
            write(video_FD, command, strlen(command));
            clear_command(command);
        }
        color+=30;
    }



    close(video_FD);
    return 0;
}

void init_command(char command[])
{
    command[0] = 'p'; command[1] = 'i'; command[2] = 'x';
    command[3] = 'e'; command[4] = 'l'; command[5] = ' ';
    command[6] = '\0';
}

void clear_command(char command[])
{
    int i = 0;
    while (command[i] != '\0')
        command[i++] = '\0';
}
