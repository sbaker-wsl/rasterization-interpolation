#include <linux/fs.h>           // struct file, struct file_operations
#include <linux/module.h>       // for module init and exit macros
#include <linux/miscdevice.h>   // for struct miscdev
#include <linux/uaccess.h>      // for copy_to_user
#include <asm/io.h>             // for mmap
#include "../address_map_arm.h"

// Declare global variables needed to use the pixel buffer
void *LW_virtual;                   // Used to access FPGA lightweight bridge
volatile int *pixel_ctrl_ptr;       // virtual address of pixel buffer controller
int pixel_buffer;                   // used for virtual address of pixel buffer;
int resolution_x, resolution_y;     // HDMI screen size

// Prototypes needed for video
void get_screen_specs(volatile int *);
void clear_screen(void);
void plot_pixel(int, int, unsigned char);
void draw_line(int, int, int, int, unsigned char);

// prototypes for commands
int atoi(char [], int);
unsigned char htoi(char []);
int absolute(int);

// Declare variables and prototypes needed for character device driver
static int video_open(struct inode *, struct file *);
static int video_release(struct inode *, struct file *);
static ssize_t video_read(struct file *, char *, size_t, loff_t *);
static ssize_t video_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations video_fops = {
    .owner = THIS_MODULE,
    .read = video_read,
    .write = video_write,
    .open = video_open,
    .release = video_release
};

#define SUCCESS 0
#define DEV_NAME "video"
#define MAX_SIZE 8
#define XY_SIZE 3

static struct miscdevice videodev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &video_fops,
    .mode = 0666
};

static int videodev_registered = 0;
static char videodev_msg[MAX_SIZE];
static char x_buffer[XY_SIZE];
static char y_buffer[XY_SIZE];
static char x1_buf[XY_SIZE];
static char x2_buf[XY_SIZE];
static char y1_buf[XY_SIZE];
static char y2_buf[XY_SIZE];
static char hex_buffer[4];

/* Code to initialize the video driver */
static int __init start_video(void) {
    // initialize the miscdevice data structures
    int err = misc_register(&videodev);
    if (err < 0) {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME);
    }
    else {
        printk(KERN_INFO "/dev/%s device registered\n", DEV_NAME);
        videodev_registered = 1;
    }

    // generate a virtual address for the FPGA lightweight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    if (LW_virtual == 0)
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");

    // create virtual memory access to the pixel buffer controller
    pixel_ctrl_ptr = (unsigned int *) (LW_virtual + PIXEL_BUF_CTRL_BASE);
    get_screen_specs(pixel_ctrl_ptr);       // determine X, Y screen size

    // create virtual memory access to the pixel buffer
    pixel_buffer = (int) ioremap_nocache(FPGA_ONCHIP_BASE, FPGA_ONCHIP_SPAN);
    if (pixel_buffer == 0)
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");

    // fill msg
    videodev_msg[0] = (resolution_x/100) + '0'; videodev_msg[1] = ((resolution_x%100)/10) + '0';
    videodev_msg[2] = (resolution_x%10) + '0'; videodev_msg[3] = ' ';
    videodev_msg[4] = (resolution_y/100) + '0'; videodev_msg[5] = ((resolution_y%100)/10) + '0';
    videodev_msg[6] = (resolution_y%10) + '0'; videodev_msg[7] = '\0';

    /* Erase the pixel buffer */
    clear_screen();
    return 0;
}

// reads the Resolution register in the pixel controller to set the global variables for resolution
void get_screen_specs(volatile int * pixel_ctrl_ptr) {
    unsigned int total = *(pixel_ctrl_ptr + 2);
    resolution_x = total & 0x0000FFFF;
    resolution_y = (total >> 16) & 0x0000FFFF;
}

// sets all pixels in the pixel buffer to the color 0 (black)
void clear_screen(void) {
    int x,y;
    for (x = 0; x < resolution_x; x++) {
        for (y = 0; y < resolution_y; y++) {
            plot_pixel(x,y,0);  // BLACK
        }
    }
}

:200
}

// NOTE: NO PROTECTION. ASSUMED CORRECT USAGE.
unsigned char htoi(char buf[]) {
    int i,n;
    n = 0;
    for (i = 2; i < 4; i++) {
        if ((buf[i] >= '0') && (buf[i] <= '9')) {
            n = 16 * n + (buf[i] - '0');
        } else {
            n = 10 + 16 * n + (buf[i] - 'A');
        }
    }
    return n;
}

void draw_line(int x1, int y1, int x2, int y2, unsigned char color) {
    int is_steep, dx, dy, error, x, y, y_step;
    is_steep = (absolute(y2-y1) > absolute(x2-x1));
    if (is_steep == 1) {
        int temp = x1; x1 = y1; y1 = temp;
        temp = x2; x2 = y2; y2 = temp;
    }
    if (x1 > x2) {
        int temp = x1; x1 = x2; x2 = temp;
        temp = y1; y1 = y2; y2 = temp;
    }
    dx = x2-x1;
    dy = absolute(y2-y1);
    error = -(dx / 2);
    y = y1;
    y_step = (y1 < y2) ? 1 : -1;
    for (x = x1; x <= x2; x++) {
        if (is_steep == 1) {
            plot_pixel(y,x,color);
        }
        else {
            plot_pixel(x,y,color);
        }
        error += dy;
        if (error >= 0) {
            y += y_step;
            error -= dx;
        }
    }
}

int absolute(int arg) {
    return (arg < 0) ? -arg : arg;
}

static int video_open(struct inode *inode, struct file *file) {
    return SUCCESS;
}

static int video_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t video_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {
    size_t bytes;
    bytes = strlen(videodev_msg) - (*offset);
    bytes = bytes > length ? length : bytes;

    if (bytes)
        if (copy_to_user(buffer, &videodev_msg[*offset], bytes) != 0)
            printk(KERN_ERR "Error: copy_to_user unsuccessful\n");
    *offset = bytes;
    return bytes;
}

static ssize_t video_write(struct file *filp, const char *buffer, size_t length, loff_t *offset) {
    size_t bytes;
    int i,j,x,y,x1,y1,x2,y2;
    unsigned char color;
    bytes = length;
    i = j = 0;
    if ((buffer[0] == 'p') && (buffer[1] == 'i') && (buffer[2] == 'x') && (buffer[3] == 'e') && (buffer[4] == 'l')) {
        for (i = 6; buffer[i] != ','; i++)
            x_buffer[i-6] = buffer[i];
        x = atoi(x_buffer, i-6); i++;
        for (j = 0; buffer[i] != ' '; j++, i++)
            y_buffer[j] = buffer[i];
        y = atoi(y_buffer, j); i++;
        for (j = 0; j < 4; i++, j++) {
            hex_buffer[j] = buffer[i];
        }
        color = htoi(hex_buffer);
        plot_pixel(x, y, color);
    } else if ((buffer[0] == 'c') && (buffer[1] == 'l') && (buffer[2] == 'e') && (buffer[3] == 'a') && (buffer[4] == 'r')) {
        clear_screen();
    } else if ((buffer[0] == 'l') && (buffer[1] == 'i') && (buffer[2] == 'n') && (buffer[3] == 'e')) {
        for (i = 5; buffer[i] != ','; i++)
            x1_buf[i-5] = buffer[i];
        x1 = atoi(x1_buf, i-5); i++;
        for (j = 0; buffer[i] != ' '; j++, i++)
            y1_buf[j] = buffer[i];
        y1 = atoi(y1_buf, j); i++;
        for (j = 0; buffer[i] != ','; j++, i++)
            x2_buf[j] = buffer[i];
        x2 = atoi(x2_buf, j); i++;
        for (j = 0; buffer[i] != ' '; j++, i++)
            y2_buf[j] = buffer[i];
        y2 = atoi(y2_buf, j); i++;
        // come back to this (add color)
        for (j = 0; j < 4; i++, j++)
            hex_buffer[j] = buffer[i];
        color = htoi(hex_buffer);
        draw_line(x1,y1,x2,y2,color);
    }
    return bytes;
}

MODULE_LICENSE("GPL");
