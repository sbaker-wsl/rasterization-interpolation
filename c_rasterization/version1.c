/* this is version 1 of the Fall Directed Study computer graphics
triangle rasterization

Version specifics:
implemented buffer for use of reading pixels out

Seth Baker */

#include <stdio.h>
#include <stdint.h>

#define ROWS 24
#define COLUMNS 80
#define WHITE 37
#define RED 31
#define GREEN 32
#define BLUE 34
#define YELLOW 33
#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE ROWS*COLUMNS

typedef struct Vector {
    int8_t x;
    int8_t y;
} Vector;

int pixel_buffer[BUFFER_SIZE];
void clean_canvas();
void plot_pixel(int8_t, int8_t, char, char);

Vector create_vector(int8_t x, int8_t y) {
    Vector new_vector;
    new_vector.x = x;
    new_vector.y = y;
    return new_vector;
}

int wedge_product(Vector v1, Vector v2) {
    return ((v1.x * v2.y) - (v2.x * v1.y));
}

Vector sub_vector(Vector v1, Vector v2) {
    return create_vector(v2.x-v1.x, v2.y-v1.y);
}

int inside_triangle(Vector p0, Vector p1, Vector p2, Vector p) {
    Vector d01 = sub_vector(p0,p1);
    Vector d12 = sub_vector(p1,p2);
    Vector d20 = sub_vector(p2,p0);
    if (wedge_product(d01, sub_vector(p0,p)) > 0) {
        if (wedge_product(d12, sub_vector(p1,p)) > 0) {
            if (wedge_product(d20, sub_vector(p2,p)) > 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void populate_buffer() {
    // create vectors to be used
    Vector p0 = create_vector(12,0);
	Vector p1 = create_vector(6,12);
	Vector p2 = create_vector(0,3);
    // now we populate our buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (inside_triangle(p0,p1,p2,create_vector(i % ROWS, i / ROWS))) {
            pixel_buffer[i] = 1;
        } else {
            pixel_buffer[i] = 0;
        }
    }
}

void read_and_display_buffer() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (pixel_buffer[i] == 1) {
            plot_pixel(i % ROWS,i / ROWS,YELLOW,'x');
        } else {
            plot_pixel(i % ROWS,i / ROWS,WHITE,'b');
        }
    }
}

void plot_pixel(int8_t x, int8_t y, char color, char c) {
    printf("\e[%2dm\e[%d;%dH%c", color, y, x, c);
    fflush(stdout);
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

int main(int argc, char *argv[]) {
    printf("\e[2J");
    populate_buffer();
    read_and_display_buffer();
    /*
    plot_pixel(12,0,RED,'*');
    plot_pixel(6,12,GREEN,'*');
	plot_pixel(0,3,BLUE,'*');
    */
    clean_canvas();
    return 0;
}
