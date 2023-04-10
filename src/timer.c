#include <stdio.h>
#include <stdbool.h>
#include "address_map_arm.h"
#include "interrupts.h"
#include "events.h"

int time_left;
bool animation_waiting;

void enable_timer_interrupt(void) {
	volatile int* timer_ptr = (int*)MPCORE_PRIV_TIMER;
	// set countdown to be from 1.8B cycles
	// on the timer (200MHz) it is 9 seconds
	*(timer_ptr) = 200E6 * 9;
	// enable interrupts, auto-reload and countdown
	*(timer_ptr + 2) = 0b111;
	struct event_t event = {E_TIMER_ENABLE, {.timer_enable = {}}};
	event_queue_push(event, "timer enabled");
}

void enable_intval_timer_interrupt(void) {
	volatile int* intval_timer_ptr = (int*)TIMER_BASE;
	// set countdown to be from 50M cycles
	// on the timer (100MHz) it is .5 seconds
	*(intval_timer_ptr + 2) = (int)(100E6 * .5) % 0x10000;
	*(intval_timer_ptr + 3) = (int)(100E6 * .5) / 0x10000;
	// enable interrupts, auto-reload and countdown
	*(intval_timer_ptr + 1) = 0b0101;
	struct event_t event = {E_TIMER_ENABLE, {.timer_enable = {}}};
	event_queue_push(event, "timer enabled");
}

/**
 * @brief Handles an interrupt from the A9 private timer
 *
 */
static void handle_timer_interrupt(void) {
	volatile int* timer_ptr = (int*)MPCORE_PRIV_TIMER;
	// reset interrupt bit by writting 1 to it
	*(timer_ptr + 3) &= 1;
	struct event_t event = {E_TIMER_RELOAD, {.timer_reload = {}}};
	event_queue_push(event, "timer reloaded");
}

static void handle_intval_timer_interrupt(void) {
	volatile int* intval_timer_ptr = (int*)TIMER_BASE;
	// reset interrupt bit by writting 1 to it
	*intval_timer_ptr = 0;
	struct event_t event = {E_INTVAL_TIMER_RELOAD, {.timer_reload = {}}};
	event_queue_push(event, "timer reloaded");
}

void enable_timer(void) {
	config_interrupt(IRQ_PRIV_TIMER, NULL, &handle_timer_interrupt);
}

void enable_intval_timer(void) {
	config_interrupt(IRQ_INTERVAL_TIMER2, NULL, &handle_intval_timer_interrupt);
}

void disable_timer(void) {
	volatile int* timer_ptr = (int*)MPCORE_PRIV_TIMER;
	// disable interrupts, auto-reload and countdown
	*(timer_ptr + 2) = 0b000;
}

void disable_intval_timer(void) {
	volatile int* intval_timer_ptr = (int*)TIMER_BASE;
	// disable interrupts, auto-reload and countdown
	*(intval_timer_ptr + 1) = 0b1000;
}
