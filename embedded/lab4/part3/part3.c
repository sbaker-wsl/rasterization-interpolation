/* write a user-level program that controls the stopwatch driver from part 2. it should run in an endless loop, and should
control the stopwatch using the pushbutton KEY and switch SW ports */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>      // for errno
#include <string.h>     // for strerror

#define IN_BYTES 2      // max number of characters to read from /dev/KEY and /dev/SW
#define MAX_BYTES 10

#define PAUSE 0
#define RUN 1

volatile int state;

int main(int argc, char *argv[]) {
    int keydev_FD, swdev_FD, ledrdev_FD, stopwatchdev_FD, ret_val;
    char keydev_buffer[IN_BYTES], swdev_buffer[IN_BYTES], ledrdev_buffer[1], stopdev_buffer[MAX_BYTES];
    int chars_read;

    if ((keydev_FD = open("/dev/KEY", O_RDONLY)) == -1) {
        printf("Error opening /dev/KEY: %s\n", strerror(errno));
        return -1;
    }
    if ((swdev_FD = open("/dev/SW", O_RDONLY)) == -1) {
        printf("Error opening /dev/SW: %s\n", strerror(errno));
        return -1;
    }
    if ((ledrdev_FD = open("/dev/LEDR", O_WRONLY)) == -1) {
        printf("Error opening /dev/LEDR: %s\n", strerror(errno));
        return -1;
    }
    if ((stopwatchdev_FD = open("/dev/stopwatch", O_RDWR)) == -1) {
        printf("Error opening /dev/stopwatch: %s\n", strerror(errno));
        return -1;
    }

    chars_read = 0;
    state = PAUSE;
    printf("paused\n");
    while (1) {
        while ((ret_val = read(keydev_FD, keydev_buffer, IN_BYTES)) != 0) {
            if (keydev_buffer[0] == '1') {
                if (state == PAUSE) {
                    state = RUN;
                    write(stopwatchdev_FD, "run", 3);
                    printf("running\n");
                } else {
                    state = PAUSE;
                    write(stopwatchdev_FD, "stop", 4);
                    printf("pausing\n");
                }
            } else if (keydev_buffer[0] == '2') {
                if (state == RUN) {
                    while ((ret_val = read(stopwatchdev_FD, stopdev_buffer, MAX_BYTES)) != 0)
                        ;
                    printf("%s", stopdev_buffer);
                } else {
                    while (chars_read < 6) {
                        while ((ret_val = read(keydev_FD, keydev_buffer, IN_BYTES)) != 0) {
                            if (keydev_buffer[0] == '2') {
                                while ((ret_val = read(swdev_FD, swdev_buffer, IN_BYTES)) != 0) {
                                    if ((swdev_buffer[0] - '0') > 9) {
                                        printf("invalid switch value\n");
                                    } else {
                                        if ((chars_read % 2) == 0) {
                                            if (swdev_buffer[0] - '0' > 5) {
                                                stopdev_buffer[chars_read] = '5';
                                            } else {
                                                stopdev_buffer[chars_read] = swdev_buffer[0];
                                            }
                                        } else {
                                            stopdev_buffer[chars_read] = swdev_buffer[0];
                                        }
                                        ledrdev_buffer[0] = stopdev_buffer[chars_read];
                                        write(ledrdev_FD, ledrdev_buffer, 1);
                                        chars_read++;
                                    }
                                }
                            }
                        }
                    }
                    stopdev_buffer[7] = stopdev_buffer[5]; stopdev_buffer[6] = stopdev_buffer[4];
                    stopdev_buffer[5] = ':'; stopdev_buffer[4] = stopdev_buffer[3];
                    stopdev_buffer[3] = stopdev_buffer[2]; stopdev_buffer[2] = ':';
                    chars_read = 0;
                    write(stopwatchdev_FD, stopdev_buffer, 8);
                }
            }
        }
    }

    close(keydev_FD);
    close(swdev_FD);
    close(ledrdev_FD);
    close(stopwatchdev_FD);

    return 0;
}
