#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>
#include "../address_map_arm.h"

#define NANOSEC 500000

/* prototype functions used to access physical memory addresses */
int open_physical(int);
void *map_physical(int, unsigned int, unsigned int);
void close_physical(int);
int unmap_physical(void *, unsigned int);
int power(int, int);

/* this program should repeatedly turn LED0 - LED7 on from right to left then left to right */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: <exe> <number of cyclces>\n");
        return 0;
    }
    const int CYCLES = atoi(argv[1]);
    volatile int * LEDR_ptr;
    int fd = -1;
    void *LW_virtual;

    // Create virtual memory access to the FPGA light-weight bridge
    if ((fd = open_physical(fd)) == -1)
        return -1;
    if (!(LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)))
        return -1;

    // set virtual address pointer to I/O port
    LEDR_ptr = (int *) (LW_virtual + LEDR_BASE);
    int i,j;
    struct timespec sleep_time;
    sleep_time.tv_sec = 1;
    sleep_time.tv_nsec = NANOSEC;
    j = 0;
    while (j++ < CYCLES) {
        for (i = 0; i < 8; i++) {
            if ((j > 1) && (i == 0))
                continue;
            *LEDR_ptr = *LEDR_ptr + power(2,i);
            nanosleep(&sleep_time, NULL);
            *LEDR_ptr = *LEDR_ptr - power(2,i);
        }
        for (i = 6; i >= 0; i--) {
            *LEDR_ptr = *LEDR_ptr + power(2,i);
            nanosleep(&sleep_time, NULL);
            *LEDR_ptr = *LEDR_ptr - power(2,i);
        }
    }
    unmap_physical(LW_virtual, LW_BRIDGE_SPAN);
    close_physical(fd);
    return 0;
}

/* Open /dev/mem to give access to physical addresses */
int open_physical(int fd)
{
    if (fd == -1)
        if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
            printf("ERROR: could not open \"/dev/mem\"...\n");
            return -1;
        }
    return fd;
}

/* Close /dev/mem to give access to physical addresses */
void close_physical (int fd) {
    close(fd);
}

/* Establish a virtual address mapping for the physical addresses starting at base and extending by span bytes */
void *map_physical(int fd, unsigned int base, unsigned int span) {
    void *virtual_base;
    virtual_base = mmap(NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if (virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() failed...\n");
        close(fd);
        return NULL;
    }
    return virtual_base;
}

/* Close the previously-opened virtual address mapping */
int unmap_physical(void *virtual_base, unsigned int span) {
    if (munmap(virtual_base, span) != 0) {
        printf("ERROR: munmap() failed...\n");
        return -1;
    }
    return 0;
}

/* basic power function using ints meant to be able to access each LED individually */
int power(int base, int exponent)
{
    int i, ret;
    ret = 1;
    for (i = 0; i < exponent; i++)
        ret*=base;
    return ret;
}
