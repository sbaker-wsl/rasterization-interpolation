#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "../address_map_arm.h"
#include "../interrupt_ID.h"

#define PAUSED 1
#define RESUME 0

void *LW_virtual;           // Lightweight bridge base address
volatile int *TIMER0_ptr;   // virtual address for timer
volatile int *KEY_ptr;      // virtual address for KEY port
volatile int *SW_ptr;       // virtual address for switch port
volatile int *LEDR_ptr;     // virtual address for LED port (testing purposes)

volatile int dseconds;      // variable for hundreth seconds
volatile int seconds;
volatile int minutes;
volatile int state;
volatile int count;

irq_handler_t stopwatch_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    if (*(KEY_ptr + 3) == 1) {      // checks edge caputer reg to see if KEY[0] has been pressed
        *(TIMER0_ptr + 1) = 0x8;    // set STOP bit
        *(KEY_ptr + 3) = 0xF;       // clear edge capture bits
        state = PAUSED;             // set our internal state variable to paused
        *LEDR_ptr = 1;              // tell user that stopwatch has been paused and is ready to set the stopwatch
    }
    while (state == PAUSED) {
        if (*(KEY_ptr + 3) == 1) {
            *(KEY_ptr + 3) = 0xF;
            *(TIMER0_ptr + 1) = 0x7;
            state = RESUME;
            count = 0;
            *LEDR_ptr = 0x80;
        } else if (*(KEY_ptr + 3) == 2) {   // checks if KEY[1] has been pressed
            count++;
            switch (count) {
                case 1:
                    dseconds = *SW_ptr;
                    if (dseconds > 9)
                        dseconds = 9;
                    *LEDR_ptr = 2;
                    break;
                case 2:
                    dseconds += (*(SW_ptr) * 10);
                    if (dseconds > 99)
                        dseconds = 99;
                    *LEDR_ptr = 4;
                    break;
                case 3:
                    seconds = *SW_ptr;
                    if (seconds > 9)
                        seconds = 9;
                    *LEDR_ptr = 8;
                    break;
                case 4:
                    seconds += (*(SW_ptr) * 10);
                    if (seconds > 59)
                        seconds = 59;
                    *LEDR_ptr = 16;
                    break;
                case 5:
                    minutes = *SW_ptr;
                    if (minutes > 9)
                        minutes = 9;
                    *LEDR_ptr = 32;
                    break;
                case 6:
                    minutes += (*(SW_ptr) * 10);
                    if (minutes > 59)
                        minutes = 59;
                    *LEDR_ptr = 64;     // tell user that stopwatch has been set and is ready to be running again
                    break;
                default:
                    printk(KERN_DEBUG "timer set for %2d:%2d:%2d\n", minutes, seconds, dseconds);
                    break;
            }
            *(KEY_ptr + 3) = 0xF;
        }
    }
    if (dseconds != 0) {    // when dseconds = 0 outside this if, the whole stopwatch should be 0
        dseconds--;
        if ((dseconds == 0) && ((seconds > 0) || (minutes > 0))) {
            if (seconds > 0) {
                seconds--;
                dseconds = 100;
            } else {
                minutes--;
                seconds = 59;
                dseconds = 100;
            }
        }
    }
    if (*(KEY_ptr + 3) == 2) {
        printk(KERN_DEBUG "%2d:%2d:%2d\n", minutes, seconds, dseconds);
        *(KEY_ptr + 3) = 0xF;
    }
    // clear interrupt
    *TIMER0_ptr = 0;
    return (irq_handler_t) IRQ_HANDLED;
}

static int __init initialize_stopwatch_handler(void)
{
    int value;
    state = RESUME;
    count = 0;
    minutes = seconds = dseconds = 0;
    // generate a virtual address for the FPGA lightweight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

    // set up virtual address mapping for led, timer0, and key
    TIMER0_ptr = LW_virtual + TIMER0_BASE;
    LEDR_ptr = LW_virtual + LEDR_BASE;
    KEY_ptr = LW_virtual + KEY_BASE;
    SW_ptr = LW_virtual + SW_BASE;

    // turn on farthest LED so that we know its on
    *LEDR_ptr = 0x80;

    // init timer0
    *(TIMER0_ptr + 2) = 0x4240;
    *(TIMER0_ptr + 3) = 0x000F;

    // starting timer
    *(TIMER0_ptr + 1) = 0x7;

    // register the interrupt handler
    value = request_irq(INTERVAL_TIMER_IRQi, (irq_handler_t) stopwatch_irq_handler, IRQF_SHARED,
        "stopwatch_irq_handler", (void *) (stopwatch_irq_handler));
    return value;
}

static void __exit cleanup_stopwatch_handler(void)
{
    *LEDR_ptr = 0;
    *(TIMER0_ptr) = 0x0;
    *(TIMER0_ptr + 1) = 0x0;
    *(TIMER0_ptr + 2) = 0x0;
    *(TIMER0_ptr + 3) = 0x0;
    *(KEY_ptr + 3) = 0xF;
    free_irq(INTERVAL_TIMER_IRQi, (void*) stopwatch_irq_handler);
}

module_init(initialize_stopwatch_handler);
module_exit(cleanup_stopwatch_handler);
