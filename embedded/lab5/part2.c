/* want to make line drawing on ascii graphics */
#include <stdio.h>
#include <stdlib.h>

#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define CYAN 36
#define WHITE 37
#define COLUMNS 132
#define LINES 115

void draw_pixel(int, int, char, char);
void draw_line(int, int, int, int, char);
void draw_canvas(void);
void cleanup_canvas(void);

int main(void)
{
    draw_canvas();
    draw_line(1,LINES,COLUMNS,1,RED);
    draw_line(1,LINES,64,1,GREEN);
    draw_line(1,LINES,COLUMNS,LINES/2,BLUE);
    draw_line(1,LINES,1,1,YELLOW);
    draw_line(1,LINES,COLUMNS,LINES-12,CYAN);
    cleanup_canvas();
    return 0;
}

void draw_pixel(int x, int y, char color, char c) {
    printf("\e[%2dm\e[%d;%dH%c", color, y, x, c);
    fflush(stdout);
}

void draw_line(int x0, int y0, int x1, int y1, char color) {
    int is_steep, dx, dy, error, x, y, y_step, temp;
    is_steep = (abs(y1-y0) > abs(x1-x0));
    if (is_steep) {
        temp = x0; x0 = y0; y0 = temp;
        temp = x1; x1 = y1; y1 = temp;
    }
    if (x0 > x1) {
        temp = x0; x0 = x1; x1 = temp;
        temp = y0; y0 = y1; y1 = temp;
    }
    dx = x1 - x0;
    dy = abs(y1 - y0);
    error = -(dx/2);
    y = y0;
    if (y0 < y1) {
        y_step = 1;
    } else {
        y_step = -1;
    }
    for (x = x0; x <= x1; x++) {
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

void draw_canvas(void) {
    int i;
    printf("\e[2J");        // clear the screen
    printf("\e[?25l");      // hide the cursor
    for (i = 1; i <= COLUMNS; i++) {
        draw_pixel(i, 1, WHITE, '*');
        draw_pixel(i, LINES, WHITE, '*');
    }
}

void cleanup_canvas(void) {
    char c;
    c = getchar();              // wait for user to press return
    printf("\e[2J");            // clear the screen
    printf("\e[%2dm", WHITE);   // reset foreground color
    printf("\e[%d;%dH", 1, 1);  // move cursor to upper left
    printf("\e[?25h");          // show the cursor
    fflush(stdout);
}
