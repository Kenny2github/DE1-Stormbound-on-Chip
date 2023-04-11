#include <stdio.h>
#include "address_map_arm.h"
#include "interrupts.h"
#include "mouse.h"
#include "timer.h"
#include "events.h"
#include "render.h"
#include "vga.h"
#include "game.h"

struct mouse_state_t saved_mouse_states[NUM_MOUSE_STATES];

int main(void) {
	configure_vga();
	enable_mouse();
	enable_timer();
	enable_intval_timer();
	config_interrupts();

	init_game();

	while (1) {
		// shift mouse states
		for (int i = NUM_MOUSE_STATES - 1; i > 0; --i) {
			saved_mouse_states[i] = saved_mouse_states[i - 1];
		}
		saved_mouse_states[0] = mouse_state;
		run_game();
		render_stack();
	}
}
