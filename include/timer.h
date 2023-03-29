/**
 * Routines for timer handling.
 */
#ifndef TIMER_H
#define TIMER_H

/**** Exported functions ****/

/**
 * @brief Enable interrupts for A9 private timer 
 */
void enable_timer_interrupt(void);

/**
 * @brief Enable timer functionality.
 *
 * This sets up interrupts and such to track time.
 */
void enable_timer(void);

/**
 * @brief Disable timer functionality.
 *
 * This disables interrupts and stops timer count down.
 */
void disable_timer(void);

#endif