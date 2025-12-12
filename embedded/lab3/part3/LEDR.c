/* lab 3 pt 3: make a kernel module that controls the LEDR lights in the DE10-Nano Computer, via the file /dev/LEDR */

#include <linux/fs.h>           // struct file, struct file_operations
#include <linux/init.h>         // for __init
#include <linux/module.h>       // for module init and exit macros
#include <linux/miscdevice.h>   // for misc_device_register and struct miscdev
#include <asm/io.h>             // for mmap
#include "../address_map_arm.h" // for virtual addresses

// file operation funcs for LEDR
static int ledr_open(struct inode *, struct file *);
static int ledr_release(struct inode *, struct file *);
static ssize_t ledr_write(struct file *, char *, size_t);

// atoi func for echo input
int htoi(char *);

// file ops struct for LEDR
static struct file_operations ledr_fops = {
    .owner = THIS_MODULE,
    .write = ledr_write,
    .open = ledr_open,
    .release = ledr_release
};

#define SUCCESS 0
#define MAXLEDR 255
#define LEDR_NAME "LEDR"

// miscdevice struct for LEDR
static struct miscdevice ledrdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = LEDR_NAME,
    .fops = &ledr_fops,
    .mode = 0666                // sets read/write access for all users
};

static int ledrdev_registered = 0;

// global vars that hold virtual addresses for lightweight bridge and LEDR port
void *LW_virtual;
volatile int *LEDR_ptr;

static int __init start_ledr_driver(void) {
    int errledr = misc_register(&ledrdev);
    if (errledr < 0) {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", LEDR_NAME);
    } else {
        printk(KERN_INFO "/dev/%s driver registered\n", LEDR_NAME);
        ledrdev_registered = 1;
    }
    // generate virtual addresses for lightweight bridge and LEDR port
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    LEDR_ptr = LW_virtual + LEDR_BASE;
    return errledr;
}

static void __exit stop_ledr_driver(void) {
    if (ledrdev_registered) {
        misc_deregister(&ledrdev);
        *LEDR_ptr = 0;
        printk(KERN_INFO "/dev/%s driver de-registered\n", LEDR_NAME);
    }
}

int htoi(char *s)
{
    /* LAB 3 STUFF
    int i, n;
    n = 0;
    for (i = 0; i < 2; ++i) {
        if (((*(s+i) - '0') <= 9) && ((*(s+i) - '0') >= 0))         // is digit
            n = 16 * n + (s[i] - '0');
        else if (((*(s+i) - 'A') <= 25) && ((*(s+i) - 'A') >= 0))   // is captial letter
            n = 10 + 16 * n + (*(s+i) - 'A');
        else                                                        // N/A return error code -1
            return -1;
    }
    return n;
    */
    /* LAB 4 STUFF */
    return (*s - '0');
}

// ledrdev funcs
static int ledr_open(struct inode *inode, struct file *file) {
    return SUCCESS;
}

static int ledr_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t ledr_write(struct file *filp, char *buffer, size_t length) {
    int num;
    size_t bytes_written = 0;
    if ((num = htoi(buffer)) == -1 || (num > MAXLEDR))      // invalid input, error has ocurred
        bytes_written = -EINVAL;            // negative error code in write for miscdevice tells process invalid argument
    else {
        bytes_written += 4;
        *LEDR_ptr = num;
    }
    return bytes_written;
}

MODULE_LICENSE("GPL");
module_init(start_ledr_driver);
module_exit(stop_ledr_driver);
