#include "title.h"
#include "deckbuilding.h"
#include "vga.h"
#include "mouse.h"
#include "states.h"
#include "events.h"
#include "event_types.h"

static const struct surface intro_surfs[] = {
	{64, 5, &stormbound},
	{38, 120, &felflares},
	{148, 120, &emerald_towers},
	{248, 120, &summon_militia},
};
#define NUM_INTRO_SURFS (sizeof(intro_surfs) / sizeof(intro_surfs[0]))

void draw_intro(void) {
	fill_screen(BACKGROUND);
	// write instructions
	write_string(0, 6, instructions_data);
	// draw logo and example cards
	for (int i = 0; i < NUM_INTRO_SURFS; ++i) {
		r_stack_push(intro_surfs[i]);
	}
	// draw mouse
	push_image(mouse_state.x, mouse_state.y, &mouse);
}

// Update the title screen and handle state transitions
void run_title(void) {
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
			// state transition on mouse click
			player_state = P1;
			init_deckbuilding();
		}
		if (event.type == E_MOUSE_MOVE) mouse_moved = true;
	}
	// rerender mouse if it has moved
	if (mouse_moved) {
		// clear old mouse positions
		for (int i = NUM_MOUSE_STATES - 1; i > 0; --i) {
			push_image(
				saved_mouse_states[i].x,
				saved_mouse_states[i].y,
				&mouse_clear
			);
		}
		// re-render affected surfaces
		for (int i = 0; i < NUM_INTRO_SURFS; ++i) {
			for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
				if (rects_collide(
					// this surf's rect
					intro_surfs[i].x, intro_surfs[i].y,
					intro_surfs[i].data->width, intro_surfs[i].data->height,
					// previous mouse surf's rect
					saved_mouse_states[j].x, saved_mouse_states[j].y,
					mouse_clear.width, mouse_clear.height
				)) r_stack_push(intro_surfs[i]);
			}
		}
	}
	// render new mouse position
	push_image(
		saved_mouse_states[0].x,
		saved_mouse_states[0].y,
		&mouse
	);
}
