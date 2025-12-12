#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "../address_map_arm.h"
#include "../interrupt_ID.h"

void * LW_virtual;          // Lightweight bridge base address
volatile int *TIMER0_ptr;   // virtual address for timer
volatile int *LEDR_ptr;     // virtual address for LEDR port
volatile int *KEY_ptr;      // virtual address for KEY port

volatile int dseconds;      // variable for hundreth seconds
volatile int seconds;
volatile int minutes;

irq_handler_t timer_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    dseconds++;
    if (dseconds == 100) {
        seconds++;
        dseconds = 0;
    }
    if (seconds == 60) {
        minutes++;
        seconds = 0;
    }
    if (minutes == 60) {
        minutes = 0;
        seconds = 0;
        dseconds = 0;
    }

    // display time to terminal
    if (*KEY_ptr == 2) {
        printk(KERN_DEBUG "%2d:%2d:%2d\n", minutes, seconds, dseconds);
        *KEY_ptr = 0;
    }
    // clear interrupt
    *TIMER0_ptr = 0;
    return (irq_handler_t) IRQ_HANDLED;
}

static int __init initialize_timer_handler(void)
{
    int value;

    // init seconds and minutes
    seconds = 0;
    minutes = 0;

    // generate a virtual address for the FPGA lightweight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

    // set up virtual memory mappings for led, timer0, and key
    TIMER0_ptr = LW_virtual + TIMER0_BASE;
    LEDR_ptr = LW_virtual + LEDR_BASE;
    KEY_ptr = LW_virtual + KEY_BASE;

    // initialize timer0 so that it has its counter bits set
    *(TIMER0_ptr + 2) = 0x4240; // set the lower bits for 1 dsecond
    *(TIMER0_ptr + 3) = 0x000F; // set the higher bits for 1 dsecond

    // START THE TIMER
    *(TIMER0_ptr + 1) = 0x7;    // set the START bit to 1 ITO bit to 1 and CONT bit to 1

    // register the interrupt handler
    value = request_irq(INTERVAL_TIMER_IRQi, (irq_handler_t) timer_irq_handler, IRQF_SHARED,
        "timer_irq_handler", (void *) (timer_irq_handler));
    return value;
}

static void __exit cleanup_timer_handler(void)
{
    *LEDR_ptr = 0;
    free_irq(INTERVAL_TIMER_IRQi, (void*) timer_irq_handler);
}

module_init(initialize_timer_handler);
module_exit(cleanup_timer_handler);
