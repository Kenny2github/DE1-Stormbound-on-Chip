#include <stdbool.h>
#include <string.h>
#include "deckbuilding.h"
#include "vga.h"
#include "mouse.h"
#include "states.h"
#include "events.h"
#include "event_types.h"
#include "game.h"
#include "turn.h"

int card_num;
bool in_deck[30];

static void db_display_card_info(int card_id) {
	if (card_id != deckbuilding_cur_display) {
		push_image(20, 156, card_data[card_id].img);
		write_string(28, 43, empty_desc_data);
		write_string(28, 43, card_data[card_id].desc);
		deckbuilding_cur_display = card_id;
	} else if (!deckbuilding_displaying) {
		push_image(20, 156, card_data[deckbuilding_cur_display].img);
		write_string(28, 43, card_data[deckbuilding_cur_display].desc);
	}
	deckbuilding_displaying = true;
}

static void clear_card_info() {
	deckbuilding_displaying = false;
	push_image(20, 156, &clear_card);
	write_string(28, 43, empty_desc_data);
}

void init_deckbuilding() {
	fill_screen(BACKGROUND);
	clear_char_screen();
	write_string(1, 1, "Cards:");
	for (int i = 0; i < 30; ++i) {
		deckbuilding_card_surfs[i] = (struct surface){(int)(i / 10) * 80, i % 10 * 12 + 12, card_selection_box[card_data[i].faction]};
		r_stack_push(deckbuilding_card_surfs[i]);
		write_string((int)(i / 10) * 20 + (20 - strlen(card_data[i].name)) / 2, i % 10 * 3 + 4, card_data[i].name);
	}
	write_string(61, 1, player_state == P1 ? "P1 deck:" : "P2 deck:");
	deckbuilding_displaying = false;
	deckbuilding_cur_display = 100;
	deckbuilding_done_displaying = false;
	card_num = 0;
	for (int i = 0; i < 30; ++i) in_deck[i] = false;
	game_state = DECK;
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
					deckbuilding_deck_surfs[card_num] = (struct surface){240, card_num * 12 + 12, card_selection_box[card_data[deck[player_state][card_num]].faction]};
					// draw card on deck
					r_stack_push(deckbuilding_deck_surfs[card_num]);
					// write card name on deck
					write_string(60 + (20 - strlen(card_data[deck[player_state][card_num]].name)) / 2, card_num * 3 + 4, card_data[deck[player_state][card_num]].name);
					// clear card in collection
					push_image((int)(idx / 10) * 80, (idx % 10) * 12 + 12, &clear_box);
					// clear name
					write_string((int)(idx / 10) * 20, (idx % 10) * 3 + 4, empty_card_name_data);
					// if deck is full, display done button
					if (++card_num == 10) {
						push_image(SCREEN_W - 61, 156, &cardbuilding_done);
						deckbuilding_done_displaying = true;
					}
				}

			} else if (saved_mouse_states[0].x >= 240 && saved_mouse_states[0].x < SCREEN_W
			 && saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < (card_num * 12 + 12)) {	// clicked on deck

				int idx = (saved_mouse_states[0].y - 12) / 12;
				int card_id = deck[player_state][idx];
				in_deck[card_id] = false; // remove card from deck
				for (int i = idx; i < card_num - 1; ++i) {	// shift deck cards down
					deck[player_state][i] = deck[player_state][i + 1];
					deckbuilding_deck_surfs[i].data = deckbuilding_deck_surfs[i + 1].data;
					// update card box
					r_stack_push(deckbuilding_deck_surfs[i]);
					// clear name
					write_string(60, i * 3 + 4, empty_card_name_data);
					// rewrite new card name
					write_string(60 + (20 - strlen(card_data[deck[player_state][i]].name)) / 2, i * 3 + 4, card_data[deck[player_state][i]].name);
				}
				// clear last item box
				push_image(240, card_num * 12, &clear_box);
				// clear last item name
				write_string(60, card_num * 3 + 1, empty_card_name_data);
				// add back item box to collection
				push_image((int)(card_id / 10) * 80, (card_id % 10) * 12 + 12, card_selection_box[card_data[card_id].faction]);
				// add back item name to collection
				write_string((int)(card_id / 10) * 20 + (20 - strlen(card_data[card_id].name)) / 2, (card_id % 10) * 3 + 4, card_data[card_id].name);
				if (card_num-- == 10) {
					push_image(SCREEN_W - 61, 156, &clear_card);
					deckbuilding_done_displaying = false;
				}

			} else if (mouse_state.x >= SCREEN_W - 61 && mouse_state.x < SCREEN_W - 20
			 && mouse_state.y >= 156 && mouse_state.y < 217 && deckbuilding_done_displaying) {
				switch(player_state) {
					case P1: // go to next player
						player_state = P2;
						init_deckbuilding();
						break;
					case P2: // go to turn state
						init_turn();
				}
			}
		} else {
			// highlight card_data
			if (saved_mouse_states[0].x >= 0 && saved_mouse_states[0].x < 240
				&& saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < 132) {

				int idx = (int)(saved_mouse_states[0].x / 80) * 10 + (int)((saved_mouse_states[0].y - 12) / 12);
				if (!in_deck[idx]) {
					db_display_card_info(idx);
				} else {
					clear_card_info();
				}


			} else if (saved_mouse_states[0].x >= 240 && saved_mouse_states[0].x < SCREEN_W
				&& saved_mouse_states[0].y >= 12 && saved_mouse_states[0].y < (card_num * 12 + 12)) {

				int card_id = deck[player_state][(int)((saved_mouse_states[0].y - 12) / 12)];
				db_display_card_info(card_id);

			} else {
				if (deckbuilding_displaying) {
					clear_card_info();
				}
			}
		}
		if (event.type == E_MOUSE_MOVE) mouse_moved = true;
	}
	if (game_state != DECK) return;
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
			if (in_deck[i]) continue;
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
		for (int i = 0; i < card_num; ++i) {
			for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
				if (rects_collide(
					// this surf's rect
					deckbuilding_deck_surfs[i].x, deckbuilding_deck_surfs[i].y,
					deckbuilding_deck_surfs[i].data->width, deckbuilding_deck_surfs[i].data->height,
					// previous mouse surf's rect
					saved_mouse_states[j].x, saved_mouse_states[j].y,
					mouse_clear.width, mouse_clear.height
				)) r_stack_push(deckbuilding_deck_surfs[i]);
			}
		}
		if (deckbuilding_displaying) {
			for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
				if (rects_collide(
					// this surf's rect
					20, 156,
					41, 61,
					// previous mouse surf's rect
					saved_mouse_states[j].x, saved_mouse_states[j].y,
					mouse_clear.width, mouse_clear.height
				)) push_image(20, 156, card_data[deckbuilding_cur_display].img);
			}
		}
		if (deckbuilding_done_displaying) {
			for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
				if (rects_collide(
					// this surf's rect
					SCREEN_W - 61, 156,
					41, 61,
					// previous mouse surf's rect
					saved_mouse_states[j].x, saved_mouse_states[j].y,
					mouse_clear.width, mouse_clear.height
				)) push_image(SCREEN_W - 61, 156, &cardbuilding_done);
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
