#include <stdbool.h>
#include "deckbuilding.h"
#include "vga.h"
#include "mouse.h"
#include "states.h"
#include "events.h"
#include "event_types.h"

int card_num;
bool in_deck[30];

void draw_deckbuilding(void) {
	fill_screen(BACKGROUND);
	clear_char_screen();
	write_string(1, 1, "Cards:");
	for (int i = 0; i < 30; ++i) {
		deckbuilding_card_surfs[i] = (struct surface){(int)(i / 10) * 80, i % 10 * 12 + 12, card_selection_box[card_data[i].faction]};
		r_stack_push(deckbuilding_card_surfs[i]);
		write_string((int)(i / 10) * 20 + (20 - strlen(card_data[i].name)) / 2, i % 10 * 3 + 4, card_data[i].name);
	}
	write_string(61, 1, "P1 deck:");
	deckbuilding_displaying = false;
	deckbuilding_done_displaying = false;
}

void run_deckbuilding(void) {
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
			if (deckbuilding_displaying) {
				deckbuilding_displaying = false;
				push_image(20, 156, &clear_card);
				write_string(28, 43, empty_desc_data);
			}
			if (card_num != 10
			 && saved_mouse_states[0].x >= 0 && saved_mouse_states[0].x < 240
			 && saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < 132) {	// clicked on available cards

				int idx = (int)(saved_mouse_states[0].x / 80) * 10 + (int)((saved_mouse_states[0].y - 12) / 12);
				if (!(in_deck[idx])) {	// move card to deck
					in_deck[idx] = true;
					deck[player_state][card_num] = idx;
					deckbuilding_deck_surfs[card_num] = (struct surface){240, card_num * 12 + 12, card_selection_box[card_data[deck[player_state][card_num++]].faction]};
					r_stack_push(deckbuilding_deck_surfs[card_num++]);
					deckbuilding_card_surfs[idx] = (struct surface){(int)(idx / 10) * 80, (idx % 10) * 12 + 12, &clear_box};
					r_stack_push(deckbuilding_card_surfs[idx]);
					if (card_num == 10) {
						push_image(SCREEN_W - 61, 156, &cardbuilding_done);
						deckbuilding_done_displaying = true;
					}
				}

			} else if (saved_mouse_states[0].x >= 240 && saved_mouse_states[0].x < SCREEN_W
			 && saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < (card_num * 12 + 12)) {	// clicked on deck

				int idx = (saved_mouse_states[0].y - 12) / 12;
				in_deck[deck[player_state][idx]] = false; // remove card from deck
				for (int i = idx; i < card_num - 1; ++i) {	// shift deck cards down
					deck[player_state][i] = deck[player_state][i + 1];
					deckbuilding_deck_surfs[i] = deckbuilding_deck_surfs[i + 1];
					r_stack_push(deckbuilding_deck_surfs[i]);
				}
				deckbuilding_deck_surfs[card_num] = (struct surface){240, card_num * 12 + 12, &clear_box};
				--card_num;

			}
		} else {
			// highlight card_data
			if (saved_mouse_states[0].x >= 0 && saved_mouse_states[0].x < 240
				&& saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < 132) {

				int idx = (int)(saved_mouse_states[0].x / 80) * 10 + (int)((saved_mouse_states[0].y - 12) / 12);
				if (!(in_deck[idx]) && idx != deckbuilding_cur_display) {
					deckbuilding_displaying = true;
					push_image(20, 156, card_data[idx].img);
					write_string(28, 43, empty_desc_data);
					write_string(28, 43, card_data[idx].desc);
				}


			} else if (saved_mouse_states[0].x >= 240 && saved_mouse_states[0].x < SCREEN_W
				&& saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < (card_num * 12 + 12)) {

				int idx = (saved_mouse_states[0].y - 12) / 12;
				if (!(in_deck[idx]) && deck[player_state][idx] != deckbuilding_cur_display) {
					deckbuilding_displaying = true;
					push_image(20, 156, card_data[deck[player_state][idx]].img);
					write_string(28, 43, empty_desc_data);
					write_string(28, 43, card_data[deck[player_state][idx]].desc);
				}

			} else {
				if (deckbuilding_displaying) {
					deckbuilding_displaying = false;
					push_image(20, 156, &clear_card);
					write_string(28, 43, empty_desc_data);
				}
			}
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
		for (int i = 0; i < NUM_CARDS; ++i) {
			for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
				if (rects_collide(
					// this surf's rect
					deckbuilding_card_surfs[i].x, deckbuilding_card_surfs[i].y,
					deckbuilding_card_surfs[i].data->width, deckbuilding_card_surfs[i].data->height,
					// previous mouse surf's rect
					saved_mouse_states[j].x, saved_mouse_states[j].y,
					mouse_clear.width, mouse_clear.height
				)) r_stack_push(deckbuilding_card_surfs[i]);
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
