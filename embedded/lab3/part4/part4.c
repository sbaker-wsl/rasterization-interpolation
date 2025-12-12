/* make use of parts 2 and 3 to implement an adder */
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define BYTES 2     // max number of characters to read from /dev/KEY and /dev/SW
#define LEDRBYTES 3 // max number of chars for ledr buffer
#define MAXSUM 255

volatile sig_atomic_t stop;
void catchSIGINT(int signum) {
    stop = 1;
}

int htoi(char c)
{
    if ((c - '0' >= 0) && (c - '0' <= 9))
        return c - '0';
    else
        return c - 'A' + 10;
}

void itoh(char buf[], int sum)
{
    sprintf(buf, "%2X", sum);
    if (isspace(buf[0]))
        buf[0] = '0';
}

int main(int argc, char *argv[])
{
    int keydev_FD, swdev_FD, ledrdev_FD, ret_val;
    char keydev_buffer[BYTES], swdev_buffer[BYTES], ledrdev_buffer[LEDRBYTES];
    int sum, secs_waited;

    // catch SIGINT from ctrl+c instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    // open the KEY device driver for reading
    if ((keydev_FD = open("/dev/KEY", O_RDONLY)) == -1) {
        printf("Error opening /dev/KEY: %s\n", strerror(errno));
        return -1;
    }

    // open the SW device driver for reading
    if ((swdev_FD = open("/dev/SW", O_RDONLY)) == -1) {
        printf("Error opening /dev/SW: %s\n", strerror(errno));
        return -1;
    }

    // open the LEDR device driver for wrtiting
    if ((ledrdev_FD = open("/dev/LEDR", O_WRONLY)) == -1) {
        printf("Error opening /dev/LEDR: %s\n", strerror(errno));
        return -1;
    }

    sum = secs_waited = 0;
    while (!stop) {
        while ((ret_val = read(keydev_FD, keydev_buffer, BYTES)) != 0) {
            if (keydev_buffer[0] == '0')
                break;
            else {
                ret_val = read(swdev_FD, swdev_buffer, BYTES);
                if ((sum + htoi(swdev_buffer[0])) > MAXSUM) {
                    printf("ERROR: can't add %c - undisplayable\n", swdev_buffer[0]);
                    break;
                }
                sum += htoi(swdev_buffer[0]);
                itoh(ledrdev_buffer, sum);
                write(ledrdev_FD, ledrdev_buffer, LEDRBYTES);
            }
        }
        sleep(1);
        secs_waited++;
    }
    close(keydev_FD);
    close(swdev_FD);
    close(ledrdev_FD);
    return 0;
}
