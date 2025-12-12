/* Seth Baker

This C file is meant for demonstration purposes of understanding of triangle rasterization principles,
specifically meant to demonstrate my ability to write a C program that can draw a triangle given 3
vertices, determining which pixels are inside or outside said triangle.

*/

#include <stdio.h>

#define ROWS 24
#define COLUMNS 80
#define WHITE 37
#define RED 31
#define GREEN 32
#define BLUE 34
#define YELLOW 33

typedef struct Vector {
	short x;
	short y;
} Vector;

void clean_canvas();
void plot_pixel(int, int, char, char);

Vector create_vector(short x, short y) {
	Vector new_vector;
	new_vector.x = x;
	new_vector.y = y;
	return new_vector;
}

int determinant(Vector v1, Vector v2) {
	return ((v1.x * v2.y) - (v2.x * v1.y));
}

/* ASSUMPTION use case is: v2 - v1 */
Vector create_edge(Vector v1, Vector v2) {
	Vector edge_vector;
	edge_vector.x = v2.x-v1.x;
	edge_vector.y = v2.y-v1.y;
	return edge_vector;
}

/* just used for testing create function */
void print_vector(Vector v) {
	printf("(%d,%d)\n", v.x, v.y);
}

/* inside triangle function ASSUMPTION: counter-clockwise rotation */
int inside_triangle(Vector v0, Vector v1, Vector v2, Vector p) {
	/* create edges for triangle given vertices */
	Vector v0v1 = create_edge(v0,v1);
	Vector v1v2 = create_edge(v1,v2);
	Vector v2v0 = create_edge(v2,v0);
	/* check determinant of the vector spanning from the "origin" of edge vector, if 
	positive its inside the triangle, else its not inside the triangle */
	if (determinant(v0v1, create_edge(v0,p)) > 0) {
		if (determinant(v1v2, create_edge(v1,p)) > 0) {
			if (determinant(v2v0, create_edge(v2,p)) > 0) {
				return 1;
			}
		}
	}
	return 0;
}

/* function to draw triangle (minimalistic) */
void draw_triangle(Vector v0, Vector v1, Vector v2) {
	/* according to stty size, the window (maximized is 47 rows by 156 cols,
	but convention is 24 rows by 80 cols so we will try that first */
	for (int i = 1; i < ROWS; i++) {
		for (int j = 1; j < COLUMNS; j++) {
			Vector testing = create_vector(i,j);
			if (inside_triangle(v0,v1,v2,testing) == 1) {
				plot_pixel(i,j,YELLOW,'x');
			} else {
				plot_pixel(i,j,WHITE,'b');
			}
		}
	}
}

/* plot pixel func from FPGAcademy lab */
void plot_pixel(int x, int y, char color, char c) {
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

int main(int argc, char *argv) {
	// create vertices 
	Vector v0 = create_vector(12,0);
	Vector v1 = create_vector(6,12);
	Vector v2 = create_vector(0,3);
	printf("\e[2J");
	// draw the triangle
	draw_triangle(v0,v1,v2);
	// highlight the vertices to inspect if its working
	plot_pixel(12,0,RED,'*');
	plot_pixel(6,12,GREEN,'*');
	plot_pixel(0,3,BLUE,'*');
	clean_canvas();
	return 0;
}
