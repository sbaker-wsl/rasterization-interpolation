#include <stdio.h>
#define YELLOW 33
#define CYAN 36
#define WHITE 37

void plot_pixel(int, int, char, char);

int main(void)
{
    char c;
    int i;
    printf("\e[2J");        // clear the screen
    printf("\e[?251");      // hide the cursor

    plot_pixel(1,1,CYAN,'X');
    plot_pixel(132,115,CYAN,'X');
    for (i = 0; i < 10; ++i)
        plot_pixel(66, i+53, YELLOW, '*');

    c = getchar();              // wait for user to press return
    printf("\e[2J");            // clear the screen
    printf("\e[%2dm", WHITE);   // reset the foreground color
    printf("\e[%d;%dH", 1, 1);  // move the cursor to upper left
    printf("\e[?25h");          // show the cursor
    fflush(stdout);

    return 0;
}

void plot_pixel(int x, int y, char color, char c) {
    printf("\e[%2dm\e[%d;%dH%c", color, y, x, c);
    fflush(stdout);
}
