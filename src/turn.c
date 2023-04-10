#include "turn.h"
#include "address_map_arm.h"
#include "vga.h"
#include "render.h"
#include "assets.h"
#include "mouse.h"
#include "states.h"
#include "events.h"
#include "event_types.h"
#include "timer.h"
#include "game.h"
#include "card_logic.h"
#include "health_status.h"

bool cur_cards_played[4];
int cur_card_selected;
int cur_card_displayed;
bool cur_card_displaying;

int moves_left;

/* change mana to new_mana, and display mana value on HEX3-0 */
void update_mana(int new_mana) {
	int seg7[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x063f};
	volatile int* HEX30_ptr = (int*)HEX3_HEX0_BASE;
	mana = new_mana;
	if (new_mana != 0) {
		int HEX_bits = 0;
		int j = 1;
		for (int i = 10; i < 10000 && new_mana != 0; i *= 10) {
			HEX_bits += seg7[(new_mana % i) / (i / 10)] * j;
			new_mana -= new_mana % i;
			j *= 0x100;
		}
		*HEX30_ptr = HEX_bits;
	} else {
		*HEX30_ptr = seg7[0];
	}
}

void update_front(void) {
	int cur_front;
	for (int i = 4; i > 0; --i) {
		cur_front = player_state == P1 ? i : 4-i;
		for (int j = 0; j < 4; ++j) {
			if (game_board[cur_front][j] != NULL
			 && game_board[cur_front][j]->player) {
				if (player_state == P1 && cur_front == 4) cur_front = 3;
				else if (player_state == P2 && cur_front == 0) cur_front = 1;
				else front[player_state] = cur_front;

				if (front[player_state] != cur_front) {
					r_stack_push(board_base_surfs[TILES]);
					board_base_surfs[FRONT_P1 + player_state].x = (cur_front - player_state) * 42 + 54;
					r_stack_push(board_base_surfs[FRONT_P1 + player_state]);
				}
				return;
			}
		}
	}
	cur_front = player_state == P1 ? 0 : 4;
	if (front[player_state] != cur_front) {
		r_stack_push(board_base_surfs[TILES]);
		board_base_surfs[FRONT_P1 + player_state].x = (cur_front - player_state) * 42 + 54;
		r_stack_push(board_base_surfs[FRONT_P1 + player_state]);
	}
}

static void t_display_card_info(int card_id) {
	if (card_id != cur_card_displayed) {
		write_string(52, 43, empty_desc_data);
		write_string(52, 43, card_data[card_id].desc);
	} else if (!cur_card_displaying) {
		write_string(52, 43, card_data[card_id].desc);
	}
	cur_card_displaying = true;
}

void init_turn() {
	fill_screen(BACKGROUND);
	clear_char_screen();
	player_state = P1;
	game_state = TURN;
	turn_state = PRETURN_BUILDING;
	move_state = CARD_EFFECT;
	row = 0;
	col = 4;
	front[P1] = 0;
	front[P2] = 4;
	// shuffle both decks
	for (int i = 0; i < 10; ++i) {
		swap_int(&deck[P1][i], &deck[P1][rand_num(0, 9)]);
		swap_int(&deck[P2][i], &deck[P2][rand_num(0, 9)]);
	}
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 4; ++j) {
			game_board[i][j] = NULL;
		}
	}
	cur_round = 0;
	update_mana(3);
	health_change_num = 0;
	status_change_num = 0;
	write_string(1, 1, "P1 turn");
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 4; ++j) {
			tile_base_surfs[i][j] = (struct surface){col2x(i), row2y(j), &empty_tile};
			r_stack_push(tile_base_surfs[i][j]);
			tile_overlay_surf_num[i][j] = 0;
		}
	}
	board_base_surfs[BASE_P1] = (struct surface){0, 12, &board_p1};
	board_base_surfs[BASE_P2] = (struct surface){265, 12, &board_p2};
	board_base_surfs[TILES] = (struct surface){55, 12, &board_tiles};
	board_base_surfs[FRONT_P1] = (struct surface){96, 16, &front_p1};
	board_base_surfs[FRONT_P2] = (struct surface){222, 16, &front_p2};
	for (int i = 0; i < 5; ++i) r_stack_push(board_base_surfs[i]);
	enable_timer_interrupt();
}

static void init_select_card(void) {
	for (int i = 0; i < 4; ++i) {
		cur_cards_played[i] = false;
		cur_card_deck_surfs[i] = (struct surface){i * 46 + 10, 166, card_data[deck[player_state][i]].img};
		r_stack_push(cur_card_deck_surfs[i]);
	}
	cur_card_selected = 100;
	cur_card_displayed = 100;
	cur_card_displaying = false;
	turn_state = SELECT_CARD;
}

static void init_card_moving(void) {
	play_card();
	moves_left = card_data[deck[player_state][cur_card_selected]].init_move;
	health_change_idx = 0;
	status_change_idx = 0;
	update_mana(mana - card_data[deck[player_state][cur_card_selected]].cost);
	cur_cards_played[cur_card_selected] = true;
	push_image(cur_card_deck_surfs[cur_card_selected].x, cur_card_deck_surfs[cur_card_selected].y, &clear_card);
	swap_int(&deck[player_state][cur_card_selected], &deck[player_state][4]);
	for (int i = 4; i < 9; ++i) swap_int(&deck[player_state][i], &deck[player_state][i+1]);
	write_string(52, 43, empty_desc_data);
	turn_state = CARD_MOVING;
	move_state = CARD_EFFECT;
	enable_intval_timer_interrupt();
}

static void run_preturn_building(void) {
	if (animation_waiting) return;

	switch (move_state) {
		case CARD_EFFECT:
			if (health_change_num == 0 && status_change_num == 0) {
				while (game_board[col][row] == NULL || game_board[col][row]->type != BUILDING) {
					if (((player_state == P1) ? ++row : --row) == 4) {
						if (player_state == P1) {
							--col;
							row = 0;
						} else {
							++col;
							row = 3;
						}
					}
					if ((col == -1 && player_state == P1) || (col == 5 && player_state == P2)) {
						turn_state = PRETURN_UNIT;
						row = (player_state == P1) ? 0 : 3;
						col = (player_state == P1) ? 4 : 0;
						break;
					}
				}
				if (turn_state == PRETURN_BUILDING) {
					if (health_change_num == 0 && status_change_num == 0) {
						start_turn_action(row, col);
						health_change_idx = 0;
						status_change_idx = 0;
					}
					change_statuses();
					if (status_change_num == 0) change_healths();
				}
			}
			break;

		case CARD_MOVE_FORWARD:
			move_state = CARD_EFFECT;
	}
}

static void run_preturn_unit(void) {
	if (animation_waiting) return;

	switch (move_state) {
		case CARD_EFFECT:
			while (game_board[col][row] == NULL || game_board[col][row]->type != UNIT) {
				if (((player_state == P1) ? ++row : --row) == 4) {
					if (player_state == P1) {
						--col;
						row = 0;
					} else {
						++col;
						row = 3;
					}
				}
				if ((col == -1 && player_state == P1) || (col == 5 && player_state == P2)) {
					init_select_card();
					break;
				}
			}
			if (turn_state == PRETURN_UNIT) {
				health_change_num = 0;
				status_change_num = 0;
				start_turn_action(row, col);
				change_healths();
				change_statuses();
			}
			break;

		case CARD_MOVE_FORWARD:
			move_forward();
			update_front();
	}
}

static void select_card_click(void) {
	if (saved_mouse_states[0].y >= 166 && saved_mouse_states[0].y < 227) {
		for (int i = 0; i < 4; ++i) {
			if (saved_mouse_states[0].x >= i * 46 + 10 && saved_mouse_states[0].x < i * 46 + 51) {
				if (i != cur_card_selected) {
					if (cur_card_selected >= 0 && cur_card_selected < 4) {
						// clear card image
						push_image(cur_card_selected * 46 + 10, 146, &clear_card);
						// redraw but slightly higher
						cur_card_deck_surfs[cur_card_selected].y = 166;
						r_stack_push(cur_card_deck_surfs[cur_card_selected]);
					}
					cur_card_selected = i;
					// clear card image
					push_image(i * 46 + 10, 166, &clear_card);
					// redraw but slightly higher
					cur_card_deck_surfs[i].y = 146;
					r_stack_push(cur_card_deck_surfs[i]);
					turn_state = PLACE_CARD;
					break;
				}

			}
		}
	}
}

static void select_card_hover(void) {
	for (int i = 0; i < 4; ++i) {
		if (rects_collide(
			// this surf's rect
			cur_card_deck_surfs[i].x, cur_card_deck_surfs[i].y,
			cur_card_deck_surfs[i].data->width, cur_card_deck_surfs[i].data->height,
			// previous mouse surf's rect
			saved_mouse_states[0].x, saved_mouse_states[0].y,
			mouse_clear.width, mouse_clear.height
		)) {
			int card_id = deck[player_state][i];
			t_display_card_info(card_id);
			return;
		}
	}
	for (int i = 0; i < COL; ++i) {
		for (int j = 0; j < ROW; ++j) {
			if (game_board[i][j] != NULL && rects_collide(
				// this surf's rect
				tile_base_surfs[i][j].x, tile_base_surfs[i][j].y,
				tile_base_surfs[i][j].data->width, tile_base_surfs[i][j].data->height,
				// previous mouse surf's rect
				saved_mouse_states[0].x, saved_mouse_states[0].y,
				mouse_clear.width, mouse_clear.height
			)) {
				int card_id = game_board[i][j]->card_id;
				t_display_card_info(card_id);
				return;
			}
		}
	}
	if (cur_card_displaying) {
		cur_card_displaying = false;
		write_string(52, 43, empty_desc_data);
	}
}

static void select_card_rerendering(void) {
	// re-render affected surfaces
	for (int i = 0; i < 4; ++i) {	// cards
		for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
			if (rects_collide(
				// this surf's rect
				cur_card_deck_surfs[i].x, cur_card_deck_surfs[i].y,
				cur_card_deck_surfs[i].data->width, cur_card_deck_surfs[i].data->height,
				// previous mouse surf's rect
				saved_mouse_states[j].x, saved_mouse_states[j].y,
				mouse_clear.width, mouse_clear.height
			)) r_stack_push(cur_card_deck_surfs[i]);
		}
	}
	for (int i = 0; i < COL; ++i) {	// troops
		for (int j = 0; j < ROW; ++j) {
			for (int k = 1; k < NUM_MOUSE_STATES; ++k) {
				if (rects_collide(
					// this surf's rect
					tile_base_surfs[i][j].x, tile_base_surfs[i][j].y,
					tile_base_surfs[i][j].data->width, tile_base_surfs[i][j].data->height,
					// previous mouse surf's rect
					saved_mouse_states[k].x, saved_mouse_states[k].y,
					mouse_clear.width, mouse_clear.height
				)) {
					r_stack_push(tile_base_surfs[i][j]);
					for (int l = 0; l < tile_overlay_surf_num[i][j]; ++l) {
						r_stack_push(tile_overlay_surfs[i][j][l]);
					}
				}
			}
		}
	}
	for (int i = 0; i < 2; ++i) {	// board
		for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
			if (rects_collide(
				// this surf's rect
				board_base_surfs[i].x, board_base_surfs[i].y,
				board_base_surfs[i].data->width, board_base_surfs[i].data->height,
				// previous mouse surf's rect
				saved_mouse_states[j].x, saved_mouse_states[j].y,
				mouse_clear.width, mouse_clear.height
			)) r_stack_push(board_base_surfs[i]);
		}
	}
	for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
		if (rects_collide(
			// this surf's rect
			board_base_surfs[TILES].x, board_base_surfs[TILES].y,
			board_base_surfs[TILES].data->width, board_base_surfs[TILES].data->height,
			// previous mouse surf's rect
			saved_mouse_states[j].x, saved_mouse_states[j].y,
			mouse_clear.width, mouse_clear.height
		)) {
			r_stack_push(board_base_surfs[TILES]);
			r_stack_push(board_base_surfs[FRONT_P1]);
			r_stack_push(board_base_surfs[FRONT_P2]);
		}
	}
}

static void run_select_card(void) {
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
			select_card_click();
		} else {
			select_card_hover();
		}
		if (event.type == E_MOUSE_MOVE) mouse_moved = true;
	}
	if (turn_state != SELECT_CARD) return;
	if (mouse_moved) {
		// clear old mouse positions
		for (int i = NUM_MOUSE_STATES - 1; i > 0; --i) {
			push_image(
				saved_mouse_states[i].x,
				saved_mouse_states[i].y,
				&mouse_clear
			);
		}
		select_card_rerendering();
	}
	// render new mouse position
	push_image(
		saved_mouse_states[0].x,
		saved_mouse_states[0].y,
		&mouse
	);
}

static void run_place_card(void) {
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
			select_card_click();
			for (int i = 0; i < COL; ++i) {
				for (int j = 0; j < ROW; ++j) {
					if (rects_collide(
						// this surf's rect
						tile_base_surfs[i][j].x, tile_base_surfs[i][j].y,
						tile_base_surfs[i][j].data->width, tile_base_surfs[i][j].data->height,
						// previous mouse surf's rect
						saved_mouse_states[0].x, saved_mouse_states[0].y,
						mouse_clear.width, mouse_clear.height
					)) {
						row = j;
						col = i;
						if (valid_play_card()) {
							init_card_moving();
							break;
						}
					}
				}
			}
		} else {
			select_card_hover();
		}
		if (event.type == E_MOUSE_MOVE) mouse_moved = true;
	}
	if (turn_state != PLACE_CARD) return;
	if (mouse_moved) {
		// clear old mouse positions
		for (int i = NUM_MOUSE_STATES - 1; i > 0; --i) {
			push_image(
				saved_mouse_states[i].x,
				saved_mouse_states[i].y,
				&mouse_clear
			);
		}
		select_card_rerendering();
	}
	// render new mouse position
	push_image(
		saved_mouse_states[0].x,
		saved_mouse_states[0].y,
		&mouse
	);
}

static void run_card_moving(void) {
	if (animation_waiting) return;

	switch (move_state) {
		case CARD_EFFECT:
			if (status_change_num == 0) {
				if (health_change_num == 0) {
					move_state = CARD_MOVE_FORWARD;
				} else {
					change_healths();
				}
			} else {
				change_statuses();
			}
			break;

		case CARD_MOVE_FORWARD:
			if (moves_left != 0) {

			}
			move_state = CARD_EFFECT;
	}
}

void run_turn(void) {
	switch(turn_state) {
		case PRETURN_BUILDING:
			run_preturn_building();
			break;

		case PRETURN_UNIT:
			run_preturn_unit();
			break;

		case SELECT_CARD:
			run_select_card();
			break;

		case PLACE_CARD:
			run_place_card();
			break;

		case CARD_MOVING:
			run_card_moving();
			break;

		case TURN_END:
			break;
	}
}
