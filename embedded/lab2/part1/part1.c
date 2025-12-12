#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../address_map_arm.h"
#include <time.h>

#define MAXCHARS 6

// placeholder functions from intel for mem management
int open_physical(int);
void *map_physical(int, unsigned int, unsigned int);
void close_physical(int);
int unmap_physical(void *, unsigned int);

// mine
void shift_left(char [], int);

// Open /dev/mem, if not already done, to give access to physical addresses
int open_physical(int fd) {
    if (fd == -1)
        if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
            printf("ERROR: could not open \"/dev/mem\"...\n");
            return -1;
        }
    return fd;
}

// Close /dev/mem to give access to physical addresses
void close_physical(int fd) {
    close(fd);
}

/* establish a virtual address mapping for the physical addresses starting at base, and extending by span bytes */
void *map_physical(int fd, unsigned int base, unsigned int span) {
    void *virtual_base;

    // get a mapping from physical addresses to virtual addresses
    virtual_base = mmap(NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if (virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() failed...\n");
        close(fd);
        return NULL;
    }
    return virtual_base;
}

// Close the previously-opened virtual address mapping
int unmap_physical(void *virtual_base, unsigned int span) {
    if (munmap(virtual_base, span) != 0) {
        printf("ERROR: munmap() failed...\n");
        return -1;
    }
    return 0;
}

// shift the contents of the character array to the left by 1 index (beginning -> end)
void shift_left(char msg[], int len)
{
    int i;
    char beg = msg[0];
    for (i = 1; i <= len; i++)
        msg[i-1] = msg[i];
    msg[len] = beg;
}

int main(void)
{
    volatile int * KEY_ptr, * SW_ptr;   // virtual address pointer to KEY and SW

    int fd = -1;            // used to open /dev/mem for access to physical addresses
    void *LW_virtual;       // used to map physical addresses for the light-weight bridge

    struct timespec delay;
    int i;
    char msg[14] = {'I', 'n', 't', 'e', 'l', ' ', 'S', 'o', 'C', ' ', 'F', 'P', 'G', 'A'};
    delay.tv_sec = 0;
    delay.tv_nsec = 500000000;

    // Create virtual memory access to the FPGA light-weight bridge
    if ((fd = open_physical(fd)) == -1)
        return -1;
    if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return -1;

    // set virtual address pointer to I/O port
    KEY_ptr = (unsigned int *) (LW_virtual + KEY_BASE);
    SW_ptr = (unsigned int *) (LW_virtual + SW_BASE);

    /* if SW[0] is set, then we go to displaying 6 characters from the message Intel SoC FPGA shifting to the left after
    each iteration. NOTE: because of how this is set up, KEY[1] key must be pressed and held, not just pressed and quickly
    released. this is due to the timing and nanosleep. if we are to make the nanosleep shorter, this would likely fix that
    but at the cost of us viewing the characters "shift" in real time*/

    printf("%d\n", *(KEY_ptr + 3));
    *(KEY_ptr + 3) = 0xF;


    while (*SW_ptr == 1) {
        //if (*KEY_ptr == 1) {
            while (*(KEY_ptr + 3) == 1) {
                printf("\e[2J\e[H");
                fflush(stdout);
                printf("----------\n| ");
                for (i = 0; i < MAXCHARS; i++) {
                    putchar(msg[i]);
                    fflush(stdout);
                }
                printf(" |\n----------\n");
                shift_left(msg, 13);
                nanosleep(&delay, NULL);
        }
        *(KEY_ptr + 3) = 0xF;

    }

    unmap_physical(LW_virtual, LW_BRIDGE_SPAN);
    close_physical(fd);

    return 0;
}
