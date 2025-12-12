#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define RED 31
#define BLUE 34
#define WHITE 37
#define GREEN 32
#define COLUMNS 80
#define LINES 24

#define UP_LEFT 0
#define DOWN_LEFT 1
#define DOWN_RIGHT 2
#define UP_RIGHT 3

#define MAX_OBJECTS 6
#define MAX_SPEED 6
#define IN_BYTES 2

#define INCREASE 1
#define DECREASE 0

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point pt;
    int direction;
    int dx;
    int dy;
} Object;

void draw_line(Point *, Point *, char);
void draw_pixel(int, int, char, char);
void clean(void);
void draw(Object[], int, int);
void set_object(Object *, int, int, int);

volatile sig_atomic_t stop;
void catchSIGINT(int);
struct timespec animation_time;
const int vert_arr[2] = {0, 3};
volatile int NUM_OBJECTS = 1; volatile int SPEED = 1;
volatile int state_s = INCREASE; volatile int state_o = INCREASE;
char key_buffer[IN_BYTES], sw_buffer[IN_BYTES];

int main(void)
{
    int i, key_FD, sw_FD;
    time_t t;
    signal(SIGINT, catchSIGINT);
    srand((unsigned) time(&t));
    if ((key_FD = open("/dev/KEY", O_RDONLY)) == -1) {
        printf("Error opening /dev/KEY: %s\n", strerror(errno));
        return -1;
    }
    if ((sw_FD = open("/dev/SW", O_RDONLY)) == -1) {
        printf("Error opening /dev/KEY: %s\n", strerror(errno));
        return -1;
    }

    Object arr[MAX_OBJECTS];
    for (i = 0; i < NUM_OBJECTS; i++)
        set_object(&(arr[i]), (rand() % COLUMNS) + 1, (rand() % LINES) + 1, (rand() % 4));
    animation_time.tv_sec = 0;
    animation_time.tv_nsec = 699999999;
    printf("\e[?25l");
    while (!stop) {
        printf("\e[H\e[2J");
        draw(arr, key_FD, sw_FD);
        nanosleep(&animation_time, NULL);
    }
    clean();
    return 0;
}

void set_object(Object *ob, int px, int py, int dir) {
    ob->pt.x = px; ob->pt.y = py;
    ob->direction = dir;
    switch (dir) {
        case UP_LEFT:
            ob->dx = ob->dy = -SPEED;
            break;
        case UP_RIGHT:
            ob->dx = SPEED; ob->dy = -SPEED;
            break;
        case DOWN_LEFT:
            ob->dx = -SPEED; ob->dy = SPEED;
            break;
        case DOWN_RIGHT:
            ob->dx = ob->dy = SPEED;
            break;
        default:
            printf("INVALID USE CASE OF OBJECT\n");
            break;
    }
}

void draw_line(Point *p1, Point *p2, char color) {
    int is_steep, dx, dy, error, y, x, y_step, temp;
    is_steep = (abs(p2->y - p1->y) > abs(p2->x - p1->x));
    Point start = *p1; Point end = *p2;
    if (is_steep) {
        temp = start.x; start.x = start.y; start.y = temp;
        temp = end.x; end.x = end.y; end.y = temp;
    }
    if (start.x > end.x) {
        temp = start.x; start.x = end.x; end.x = temp;
        temp = start.y; start.y = end.y; end.y = temp;
    }
    dx = end.x - start.x;
    dy = abs(end.y - start.y);
    error = -(dx / 2);
    y = start.y;
    y_step = (start.y < end.y) ? 1 : -1;
    for (x = start.x; x <= end.x; x++) {
        if (is_steep) {
            draw_pixel(y,x,color,'|');
        } else {
            draw_pixel(x,y,color,'*');
        }
        error += dy;
        if (error >= 0) {
            y += y_step;
            error -= dx;
        }
    }
}

void draw_pixel(int x, int y, char color, char c) {
    printf("\e[%2dm\e[%d;%dH%c", color, y, x, c);
}

void clean() {
    printf("\e[2J");
    printf("\e[%2dm", WHITE);
    printf("\e[%d;%dH", 1, 1);
    printf("\e[?25h");
    fflush(stdout);
}

void draw(Object objects[], int key_FD, int sw_FD) {
    int i, j, ret_val;
    char color;
    while ((ret_val = read(sw_FD, sw_buffer, IN_BYTES != 0))) {
        if (sw_buffer[0] == '0') {
            for (i = 0; i < NUM_OBJECTS - 1; i++) {
                draw_line(&(objects[i].pt), &(objects[i+1].pt), color);
                color = RED + i;
            }
        }
    }
    for (i = 0; i < NUM_OBJECTS; ++i) {
        while ((ret_val = read(key_FD, key_buffer, IN_BYTES))) {
            if (key_buffer[0] == '1') {
                if (SPEED == MAX_SPEED) {
                    state_s = DECREASE;
                } else if (SPEED == 1) {
                    state_s = INCREASE;
                }
                if (state_s == DECREASE) {
                    SPEED--;
                } else {
                    SPEED++;
                }
                for (j = 0; j < NUM_OBJECTS; j++) {
                    set_object(&(objects[j]), objects[j].pt.x, objects[j].pt.y, objects[j].direction);
                }
            } else if (key_buffer[0] == '2') {
                if (NUM_OBJECTS == MAX_OBJECTS) {
                    state_o = DECREASE;
                } else if (NUM_OBJECTS == 1) {
                    state_o = INCREASE;
                }
                if (state_o == DECREASE) {
                    NUM_OBJECTS--;
                } else {
                    set_object(&(objects[NUM_OBJECTS]), (rand() % COLUMNS) + 1, (rand() % LINES) + 1, (rand() % 4));
                    NUM_OBJECTS++;
                }
            }
        }
        draw_pixel(objects[i].pt.x, objects[i].pt.y, WHITE, 'X');
        if (objects[i].pt.x + objects[i].dx < 1 || objects[i].pt.x + objects[i].dx > COLUMNS) {
            objects[i].dx = -objects[i].dx;
        }
        if (objects[i].pt.y + objects[i].dy < 1 || objects[i].pt.y + objects[i].dy > LINES) {
            objects[i].dy = -objects[i].dy;
        }
        objects[i].pt.x += objects[i].dx;
        objects[i].pt.y += objects[i].dy;

    }
    fflush(stdout);
}

void catchSIGINT(int signum) {
    stop = 1;
}
