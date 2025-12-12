/* part 1: write a character driver that implements a stopwatch. the stopwatch should use the format MM:SS:DD.
The code should initialize the stopwatch time to 59:59:99 and should decrement the time each 1/100 seconds. it
should provide the current stopwatch time via the file /dev/stopwatch - when the time reaches 00:00:00 the
stopwatch should halt.

part 2: implement write functionality such that the user can control the stopwatch via writing commands to the file
/dev/stopwatch, stop, run and MM:SS:DD */

#include <linux/fs.h>           // struct file, struct file_operations
#include <linux/init.h>         // for __init
#include <linux/module.h>       // for module init and exit macros
#include <linux/miscdevice.h>   // for misc_device_register and struct miscdev
#include <linux/uaccess.h>      // for copy_to_user
#include <asm/io.h>             // for mmap
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include "../address_map_arm.h"
#include "../interrupt_ID.h"

// file operation funcs for stopwatch
static int stopwatch_open(struct inode *, struct file *);
static int stopwatch_release(struct inode *, struct file *);
static ssize_t stopwatch_read(struct file *, char *, size_t);
static ssize_t stopwatch_write(struct file *, const char *, size_t);

// file ops struct for stopwatch
static struct file_operations stopwatch_fops = {
    .owner = THIS_MODULE,
    .read = stopwatch_read,
    .write = stopwatch_write,
    .open = stopwatch_open,
    .release = stopwatch_release
};

#define SUCCESS 0
#define STOPWATCH_NAME "stopwatch"
#define MAX_MINS 59
#define MAX_SECS 59
#define MAX_DSECS 99
#define MAX_BYTES 10
#define TRUE 1
#define FALSE 0

static struct miscdevice stopwatchdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = STOPWATCH_NAME,
    .fops = &stopwatch_fops,
    .mode = 0666
};

static int stopwatchdev_registered = 0;
static char dev_msg[MAX_BYTES];

// global vars that hold virtual addresses
void *LW_virtual;                       // lightweight bridge base address
volatile int *TIMER0_ptr;       // virtual address for interval timer
volatile int dseconds;
volatile int seconds;
volatile int minutes;
volatile int displayed;

// helper functions
void populate_msg(void) {
    dev_msg[2] = dev_msg[5] = ':';
    dev_msg[9] = '\0';
    dev_msg[0] = (minutes / 10) + '0';
    dev_msg[1] = (minutes % 10) + '0';
    dev_msg[3] = (seconds / 10) + '0';
    dev_msg[4] = (seconds % 10) + '0';
    dev_msg[6] = (dseconds / 10) + '0';
    dev_msg[7] = (dseconds % 10) + '0';
    dev_msg[8] = '\n';
}

int is_digit(char c) {
    return ((c - '0' >= 0) && (c - '0' <= 9)) ? TRUE : FALSE;
}

int is_time(const char *buffer) {
    int i;
    for (i = 0; i < (MAX_BYTES-2); i++) {
        if ((i == 2) || (i == 5)) {
            if (*(buffer + i) != ':') {
                return FALSE;
            }
        } else {
            if (is_digit(*(buffer + i)) == FALSE) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void convert_time(const char *buffer) {
    int c;
    minutes = ((c = (*(buffer + 0) - '0')) > 5) ? MAX_MINS : (c*10) + (*(buffer + 1) - '0');
    seconds = ((c = (*(buffer + 3) - '0')) > 5) ? MAX_SECS : (c*10) + (*(buffer + 4) - '0');
    dseconds = ((*(buffer + 6) - '0') * 10) + ((*(buffer + 7) - '0'));
}

// irq handler for interrupt when 1/100th second has passed
irq_handler_t stopwatch_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
    // handle seconds. when stopwatch is done, timer is set to STOP, and will no longer generate interrupts
    dseconds = (--dseconds < 0) ? 0 : dseconds;
    if (dseconds == 0) {
        if (seconds != 0) {
            seconds--;
            dseconds = MAX_DSECS;
        } else {
            if (minutes == 0) {     // stop the timer.
                *(TIMER0_ptr + 1) = 0x8;
            } else {
                minutes--;
                seconds = MAX_SECS;
                dseconds = MAX_DSECS;
            }
        }
    }
    // clear interrupts
    *TIMER0_ptr = 0;
    return (irq_handler_t) IRQ_HANDLED;
}

static int __init init_stopwatch_driver(void) {     // and handler
    // driver portion
    int value, err;
    err = misc_register(&stopwatchdev);
    if (err < 0) {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", STOPWATCH_NAME);
    } else {
        printk(KERN_INFO "/dev/%s driver registered\n", STOPWATCH_NAME);
        stopwatchdev_registered = 1;
    }
    // generate a virtual address for the FPGA lightweight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    TIMER0_ptr = LW_virtual + TIMER0_BASE;

    // interrupt handler portion
    minutes = seconds = dseconds = 0;

    // init timer0
    *(TIMER0_ptr + 2) = 0x4240; // lower 16 bits of count
    *(TIMER0_ptr + 3) = 0x000F; // upper 16 bits of count

    // start timer
    *(TIMER0_ptr + 1) = 0x7;

    // for read function, displayed makes it such that we dont have infinite reads it just updates it with the value when
    // called
    displayed = FALSE;

    // register the interrupt handler
    value = request_irq(INTERVAL_TIMER_IRQi, (irq_handler_t) stopwatch_irq_handler, IRQF_SHARED,
        "stopwatch_irq_handler", (void *) (stopwatch_irq_handler));
    return value;
}

static void __exit stop_driver(void) {
    if (stopwatchdev_registered) {
        misc_deregister(&stopwatchdev);
        printk(KERN_INFO "/dev/%s driver de-registered\n", STOPWATCH_NAME);
    }
    *(TIMER0_ptr) = 0x0;
    *(TIMER0_ptr + 1) = 0x0;
    *(TIMER0_ptr + 2) = 0x0;
    *(TIMER0_ptr + 3) = 0x0;
    free_irq(INTERVAL_TIMER_IRQi, (void*) stopwatch_irq_handler);
}

static int stopwatch_open(struct inode *inode, struct file *file) {
    return SUCCESS;
}

static int stopwatch_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t stopwatch_read(struct file *filp, char *buffer, size_t length) {
    size_t bytes;
    if (displayed == FALSE) {
        populate_msg();
        for (bytes = 0; bytes < MAX_BYTES; bytes++)
            put_user(dev_msg[bytes], buffer++);
        displayed = TRUE;
    } else {
        displayed = FALSE;
        bytes = 0;
    }
    return bytes;
}

// part 2
static ssize_t stopwatch_write(struct file *filp, const char *buffer, size_t length) {
    int i;
    if (length > MAX_BYTES - 1) {
        printk(KERN_ERR "Failed to write to /dev/%s : unknown command\n", STOPWATCH_NAME);
        return 1;   // false return, just pretends to have written bytes to continue running but tells kernel
    } else {
        if ((buffer[0] == 'r') && (buffer[1] == 'u') && (buffer[2] == 'n')) {
            *(TIMER0_ptr + 1) = 0x7;    // start timer again
        } else if ((buffer[0] == 's') && (buffer[1] == 't') && (buffer[2] == 'o') && (buffer[3] == 'p')) {
            *(TIMER0_ptr + 1) = 0x8;    // stop timer (pause)
        } else if (is_time(buffer)) {
            convert_time(buffer);       // set minutes, seconds, dseconds appropriately
        } else {
            printk(KERN_ERR "unimplemented command\n");
            return 1;
        }
        for (i = 0; i < length; i++)
            dev_msg[i] = buffer[i];
        dev_msg[i] = '\0';
        return length;
    }
}

MODULE_LICENSE("GPL");
module_init(init_stopwatch_driver);
module_exit(stop_driver);
