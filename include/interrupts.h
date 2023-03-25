/**
 * Routines for configuring interrupts.
 * This only includes the routines necessary to be exported.
 */
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/**** Interrupt IDs ****/

#define IRQ_KEY				73	// Push buttons
#define IRQ_JTAG_UART		80	// JTAG UART
#define IRQ_PRIV_TIMER		29	// Cortex-A9 Private Timer
#define IRQ_WATCH_TIMER		30	// Cortex-A9 Watchdog Timer
#define IRQ_HPS_TIMER3		203	// HPS L4 Watchdog Timer (203)
#define IRQ_HPS_TIMER4		204	// HPS L4 Watchdog Timer (204)
#define IRQ_INTERVAL_TIMER2	72	// Interval Timer (72)
#define IRQ_INTERVAL_TIMER4	74	// Interval Timer (74)
#define IRQ_CAR_UART		75	// Carworld UART
#define IRQ_PARALLEL_PORT1	11	// Parallel port (11)
#define IRQ_PARALLEL_PORT2	12	// Parallel port (12)
#define IRQ_AUDIO			78	// Audio (8 kHz)
#define IRQ_PS2				79	// PS/2 keyboard or mouse
#define IRQ_PS2_DUAL		89	// PS/2 dual keyboard and/or mouse

/**** Exported functions ****/

/**
 * @brief Configure interrupts.
 * This should be called once, as early as possible.
 */
void config_interrupts(void);

/**
 * @brief Configure a handler for an interrupt.
 *
 * @param interrupt_id The interrupt ID to handle.
 * @param enable The I/O device interrupt enabler function pointer.
 * @param handler The interrupt handler function pointer.
 */
void config_interrupt(int interrupt_id, void (*enable)(void), void (*handler)(void));

#endif
