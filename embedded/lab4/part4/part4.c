/* implement a game that uses the character device drivers to communicate with the SW switches, KEY pushbuttons, LED lights
and stopwatch. */

#include <stdio.h>          // for printf
#include <stdlib.h>         // for rand(), srand(), atoi
#include <time.h>           // useful for generating seed to be truly random
#include <errno.h>          // for errno
#include <string.h>         // for strerror
#include <fcntl.h>          // for open()
#include <unistd.h>         // for tiny sleep to indicate to user what was put into the time buffer

#define MAX_SIZE 100        // maxsize for input, should never actually excede 4 digits but if we are allowing bad answers
#define TRUE 1
#define FALSE 0
#define IN_BYTES 2          // for SW buffer and KEY buffer
#define MAX_BYTES 10        // for stopwatch buffer
#define PAUSE 0
#define RUN 1

char guess_buffer[MAX_SIZE];    // for user guess

void set_default_time(char buffer[])
{
    buffer[0] = buffer[1] = buffer[4] = buffer[6] = buffer[7] = '0';
    buffer[2] = buffer[5] = ':'; buffer[3] = '1';
}

void fix_time_buffer(char buffer[])
{
    buffer[7] = buffer[5]; buffer[6] = buffer[4]; buffer[5] = ':';
    buffer[4] = buffer[3]; buffer[3] = buffer[2]; buffer[2] = ':';
}

int main(int argc, char *argv[])
{
    // file reading variables
    int key_FD, sw_FD, led_FD, stopwatch_FD, ret_val, chars_read;
    char key_buffer[IN_BYTES], sw_buffer[IN_BYTES], led_buffer[1], stopwatch_buffer[MAX_BYTES], time_buffer[MAX_BYTES];
    // game variables
    int round_correct, state, continue_game, c, i, op1, op2, correct, custom_time;
    time_t t;
    // try opening character device driver files
    if ((key_FD = open("/dev/KEY", O_RDONLY)) == -1) {
        printf("Error opening /dev/KEY: %s\n", strerror(errno));
        return -1;
    }
    if ((sw_FD = open("/dev/SW", O_RDONLY)) == -1) {
        printf("Error opening /dev/SW: %s\n", strerror(errno));
        return -1;
    }
    if ((led_FD = open("/dev/LEDR", O_WRONLY)) == -1) {
        printf("Error opening /dev/LEDR: %s\n", strerror(errno));
        return -1;
    }
    if ((stopwatch_FD = open("/dev/stopwatch", O_RDWR)) == -1) {
        printf("Error opening /dev/stopwatch: %s\n", strerror(errno));
        return -1;
    }

    // set up game variables
    srand((unsigned) time(&t));
    continue_game = correct = TRUE;
    round_correct = chars_read = 0;
    custom_time = FALSE;

    // start by setting an appropriate time to the stopwatch, in our case, default will be 30 secs, and pause stopwatch
    state = PAUSE;
    write(stopwatch_FD, "stop", 4);
    printf("Set stopwatch if desired. Press KEY0 to start\n");
    // do-while: main loop should simply be questioning whether there was a response from user
    do {
        while ((ret_val = read(stopwatch_FD, stopwatch_buffer, MAX_BYTES)) != 0) {
            if (state == RUN) {
                if ((strcmp(stopwatch_buffer, "00:00:00\n")) == 0) {
                    continue_game = FALSE;
                    printf("You answered %d questions!\n", round_correct);
                    break;
                } else {
                    i = 0;
                    if (correct == TRUE) {
                        if (round_correct < 5) {
                            op1 = rand() % 10; op2 = rand() % 10;
                        } else if (round_correct < 10) {
                            op1 = rand() % 100; op2 = rand() % 100;
                        } else if (round_correct < 20) {
                            op1 = rand() % 1000; op2 = rand() % 100;
                        } else {
                            op1 = rand() % 1000; op2 = rand() % 1000;
                        }
                        printf("%d + %d = ", op1, op2);
                        write(stopwatch_FD, time_buffer, 8);
                    } else {
                        printf("Try again: ");
                    }
                    while ((c = getchar()) != '\n') {
                        guess_buffer[i++] = c;
                    }
                    guess_buffer[i] = '\0';
                    if ((atoi(guess_buffer)) == (op1 + op2)) {
                        correct = TRUE;
                        round_correct++;
                    } else {
                        correct = FALSE;
                    }
                }
            } else {
                while ((ret_val = read(key_FD, key_buffer, IN_BYTES)) != 0) {
                    if (key_buffer[0] == '1') { // KEY0 pressed, start game
                        if (custom_time == FALSE) {
                            set_default_time(time_buffer);
                        }
                        write(stopwatch_FD, time_buffer, 8); // to pass first strcmp when game is started
                        state = RUN;
                        write(stopwatch_FD, "run", 3);
                    } else if (key_buffer[0] == '2') {
                        // read first switch value
                        while ((ret_val = read(sw_FD, sw_buffer, IN_BYTES)) != 0) {
                            if (sw_buffer[0] - '0' > 9) {
                                time_buffer[chars_read] = '9';
                            } else {
                                time_buffer[chars_read] = sw_buffer[0];
                            }
                            led_buffer[0] = time_buffer[chars_read++];
                            write(led_FD, led_buffer, 1);
                        }
                        while (chars_read < 6) {
                            while ((ret_val = read(key_FD, key_buffer, IN_BYTES)) != 0) {
                                if (key_buffer[0] == '2') {
                                    while ((ret_val = read(sw_FD, sw_buffer, IN_BYTES)) != 0) {
                                        if ((chars_read % 2) == 0) {
                                            if (sw_buffer[0] - '0' > 5) {
                                                time_buffer[chars_read] = '5';
                                            } else {
                                                time_buffer[chars_read] = sw_buffer[0];
                                        }
                                        } else {
                                            if (sw_buffer[0] - '0' > 9) {
                                                time_buffer[chars_read] = '9';
                                            } else {
                                                time_buffer[chars_read] = sw_buffer[0];
                                            }
                                        }
                                        led_buffer[0] = time_buffer[chars_read++];
                                        write(led_FD, led_buffer, 1);
                                    }
                                }
                            }
                        }
                        custom_time = TRUE;
                        fix_time_buffer(time_buffer);
                        sleep(1);
                        led_buffer[0] = '0';
                        write(led_FD, led_buffer, 1);
                    }
                }
            }
        }
    } while (continue_game == TRUE);

    close(key_FD);
    close(sw_FD);
    close(led_FD);
    close(stopwatch_FD);

    return 0;
}
