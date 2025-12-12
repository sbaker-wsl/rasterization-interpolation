/* lab 3 pt 2: kernel module that reads the edgecapture register of KEY and the data register of SW on DE-10 nano
and writes that value to /dev/SW and /dev/KEY when the kernel module is inserted. */

#include <linux/fs.h>           // struct file, struct file_operations
#include <linux/init.h>         // for __init
#include <linux/module.h>       // for module init and exit macros
#include <linux/miscdevice.h>   // for misc_device_register and struct miscdev
#include <linux/uaccess.h>      // for copy_to_user
#include <asm/io.h>             // for mmap
#include "../address_map_arm.h" // for virtual addresses

// file operation funcs for SW
static int sw_open(struct inode *, struct file *);
static int sw_release(struct inode *, struct file *);
static ssize_t sw_read(struct file *, char *, size_t);

// file operation funcs for KEY (todo)
static int key_open(struct inode *, struct file *);
static int key_release(struct inode *, struct file *);
static ssize_t key_read(struct file *, char *, size_t);

// file ops struct for SW
static struct file_operations sw_fops = {
    .owner = THIS_MODULE,
    .read = sw_read,
    .open = sw_open,
    .release = sw_release
};

// file ops struct for KEY
static struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .read = key_read,
    .open = key_open,
    .release = key_release
};

#define SUCCESS 0
#define SW_NAME "SW"
#define KEY_NAME "KEY"

static struct miscdevice swdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = SW_NAME,
    .fops = &sw_fops,
    .mode = 0666
};

static struct miscdevice keydev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = KEY_NAME,
    .fops = &key_fops,
    .mode = 0666        // sets read/write access for all users
};

static int swdev_registered = 0;
static int keydev_registered = 0;

// global vars that hold virtual addresses for KEY and SW ports
void *LW_virtual;               // lightweight bridge base address
volatile int *KEY_ptr, *SW_ptr; // virtual addresses for KEY and SW ports
volatile int swprev;
volatile int keyprev;

static int __init init_drivers(void) {
    int errsw = misc_register(&swdev);      // register swdev as misc device
    int errkey = misc_register(&keydev);    // register keydev as misc device
    if (errsw < 0) {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", SW_NAME);
    } else {
        printk(KERN_INFO "/dev/%s driver registered\n", SW_NAME);
        swdev_registered = 1;
    }
    if (errkey < 0) {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", KEY_NAME);
    } else {
        printk(KERN_INFO "/dev/%s driver registered\n", KEY_NAME);
        keydev_registered = 1;
    }
    // generate a virtual address for the FPGA lightweight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    KEY_ptr = LW_virtual + KEY_BASE;    // virtual address for KEY port;
    SW_ptr = LW_virtual + SW_BASE;      // virtual address for SW port;

    // set the previous value of SW to -1 (this way, it will always be false for any value inside the pointer to SW
    swprev = keyprev = -1;
    return errsw;
}

static void __exit stop_drivers(void) {
    if (swdev_registered) {
        misc_deregister(&swdev);
        printk(KERN_INFO "/dev/%s driver de-registered\n", SW_NAME);
    }
    if (keydev_registered) {
        misc_deregister(&keydev);
        printk(KERN_INFO "/dev/%s driver de-registered\n", KEY_NAME);
    }
}

// swdev funcs
static int sw_open(struct inode *inode, struct file *file) {
    return SUCCESS;
}

static int sw_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t sw_read(struct file *filp, char *buffer, size_t length) {
    size_t bytes_read = 0;
    if (swprev == *(SW_ptr)) {
        swprev = -1;            // re-set prev to -1 so it will still put char into user space even if SW is same from prev cat
        return bytes_read;      // will return 0 to signal EOF
    }
    else {
        if (*(SW_ptr) < 10) {
            put_user(*(SW_ptr) + '0', buffer);
            put_user('\n', ++buffer);
        }
        else {
            put_user((*(SW_ptr) - 10) + 'A', buffer);
            put_user('\n', ++buffer);
        }
        bytes_read += 2;
        swprev = *(SW_ptr);
    }
    return bytes_read;  // bytes read
}

// keydev funcs
static int key_open(struct inode *inode, struct file *file) {
    return SUCCESS;
}

static int key_release(struct inode *inode, struct file *file) {
    *(KEY_ptr + 3) = 0xF;
    return 0;
}

static ssize_t key_read(struct file *filp, char *buffer, size_t length) {
    size_t bytes_read = 0;
    if (*(KEY_ptr + 3) == 0) {

    } else {
        put_user(*(KEY_ptr + 3) + '0', buffer);
        put_user('\n', ++buffer);
        bytes_read += 2;
        *(KEY_ptr + 3) = 0xF;
    }
    return bytes_read;
}

MODULE_LICENSE("GPL");
module_init(init_drivers);
module_exit(stop_drivers);
