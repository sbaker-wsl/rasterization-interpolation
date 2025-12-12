/* animation of line up and down on the screen */

#include <stdio.h>
#include <time.h>

#define COLUMNS 132
#define LINES 115
#define YELLOW 33
#define WHITE 37

#define UP 1
#define DOWN 0

void draw_line(int);
void draw_canvas(void);
void plot_pixel(int, int, char, char);
void clean_canvas(void);

int main(void)
{
    int direction, position, i;
    struct timespec tim1;
    tim1.tv_sec = 0;
    tim1.tv_nsec = 500000000;
    direction = UP;
    position = LINES/2;
    for (i = 0; i < (LINES*2); i++) {
        if (position == 1) {
            direction = DOWN;
        } else if (position >= LINES) {
            direction = UP;
        }
        printf("\e[2J");
        draw_line(position);
        if (direction == UP) {
            position--;
        } else {
            position++;
        }
        nanosleep(&tim1, NULL);
    }
    clean_canvas();
    return 0;
}

void plot_pixel(int x, int y, char color, char c) {
    printf("\e[%2dm\e[%d;%dH%c", color, y, x, c);
    fflush(stdout);
}

void draw_line(int pos) {
    int i;
    for (i = 0; i < 10; i++) {
        plot_pixel((COLUMNS/2 - 5) + i, pos, YELLOW,  '*');
        fflush(stdout);
    }
}

void draw_canvas(void) {
    int i;
    printf("\e[2J");
    printf("\e[?25l");
    for (i = 1; i <= COLUMNS; i++) {
        plot_pixel(i, 1, WHITE, '-');
        plot_pixel(i, LINES, WHITE, '-');
    }
}

void clean_canvas() {
    char c;
    c = getchar();
    printf("\e[2J");
    printf("\e[%2dm", WHITE);
    printf("\e[%d;%dH", 1, 1);
    printf("\e[?25h");
    fflush(stdout);
}
