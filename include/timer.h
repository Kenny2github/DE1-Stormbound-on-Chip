/**
 * Routines for timer handling.
 */
#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

/**** Global variables ****/

/**
 * @brief Current time left in a turn
 */
extern int time_left;

/**
 * @brief Whether game is waiting for animation to end
 */
extern bool animation_waiting;

/**** Exported functions ****/

/**
 * @brief Enable interrupts for A9 private timer
 */
void enable_timer_interrupt(void);

/**
 * @brief Enable interrupts for interval timer
 */
void enable_intval_timer_interrupt(void);

/**
 * @brief Enable timer functionality.
 *
 * This sets up interrupts and such to track time.
 */
void enable_timer(void);

/**
 * @brief Enable interval timer functionality.
 *
 * This sets up interrupts and such to track time.
 */
void enable_intval_timer(void);

/**
 * @brief Disable timer functionality.
 *
 * This disables interrupts and stops timer count down.
 */
void disable_timer(void);

/**
 * @brief Disable interval timer functionality.
 *
 * This disables interrupts and stops timer count down.
 */
void disable_intval_timer(void);

#endif
