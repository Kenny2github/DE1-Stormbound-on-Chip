#include "turn.h"
#include "address_map_arm.h"
#include "vga.h"
#include "render.h"
#include "mouse.h"
#include "states.h"
#include "events.h"
#include "event_types.h"
#include "timer.h"
#include "game.h"
#include "health_status.h"

bool cur_cards_played[4];
int cur_card_selected;

/* change mana to new_mana, and display mana value on HEX3-0 */
void update_mana(int new_mana) {
	int seg7[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x063f};
	volatile int* HEX30_ptr = (int*)HEX3_HEX0_BASE;
	int HEX_bits = 0;
	mana = new_mana;
	int j = 1;
	for (int i = 10; i < 10000 && new_mana != 0; i *= 10) {
		if (new_mana < i) {
			HEX_bits += seg7[(new_mana % i) / (i / 10)] *= j;
			new_mana -= new_mana % i;
			j *= 0x100;
		}
	}
	*HEX30_ptr = HEX_bits;
}

void update_front(void) {
	for (int i = 4; i > 0; --i) {
		int cur_front = player_state == P1 ? i : 4-i;
		for (int j = 0; j < 4; ++j) {
			if (game_board[cur_front][j] != NULL
			 && game_board[cur_front][j]->player) {
				if (player_state == P1 && cur_front == 4) front[player_state] = 3;
				else if (player_state == P2 && cur_front == 0) front[player_state] = 1;
				else front[player_state] = cur_front;
				return;
			}
		}
	}
	front[player_state] = player_state == P1 ? 0 : 4;
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
			board_base_surfs[i][j] = (struct surface){col2x(i), row2y(j), &empty_tile};
			r_stack_push(board_base_surfs[i][j]);
			board_overlay_surf_num[i][j] = 0;
		}
	}
	board_base_surfs[5][0] = (struct surface){0, 12, &board_p1};
	board_base_surfs[6][0] = (struct surface){55, 12, &board_tiles};
	board_base_surfs[7][0] = (struct surface){265, 12, &board_p2};
	for (int i = 0; i < 3; ++i) r_stack_push(board_base_surfs[i+5][0]);
	enable_timer_interrupt();
	printf("init done\n");
}

static void init_select_card(void) {
	for (int i = 0; i < 4; ++i) {
		cur_cards_played[i] = false;
		cur_card_deck_surfs[i] = (struct surface){i * 46 + 10, 166, card_data[deck[player_state][i]].img};
		r_stack_push(cur_card_deck_surfs[i]);
	}
	turn_state = SELECT_CARD;
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

static void run_select_card(void) {
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
			if (mouse_state.y >= 166 && mouse_state.y < 227) {
				for (int i = 0; i < 4; ++i) {
					if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
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
		bool on_tile = false;
		for (int i = 0; i < COL; ++i) {	// troops
			for (int j = 0; j < ROW; ++j) {
				for (int k = 1; k < NUM_MOUSE_STATES; ++k) {
					if (rects_collide(
						// this surf's rect
						board_base_surfs[i][j].x, board_base_surfs[i][j].y,
						board_base_surfs[i][j].data->width, board_base_surfs[i][j].data->height,
						// previous mouse surf's rect
						saved_mouse_states[k].x, saved_mouse_states[k].y,
						mouse_clear.width, mouse_clear.height
					)) {
						r_stack_push(board_base_surfs[i][j]);
						for (int l = 0; l < board_overlay_surf_num[i][j]; ++l) {
							r_stack_push(board_overlay_surfs[i][j][l]);
						}
						on_tile = true;
					}
				}
			}
		}
		for (int i = 5; i < 8; ++i) {	// board
			for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
				if (!(i == 6 && !on_tile) && rects_collide(
					// this surf's rect
					board_base_surfs[i][0].x, board_base_surfs[i][0].y,
					board_base_surfs[i][0].data->width, board_base_surfs[i][0].data->height,
					// previous mouse surf's rect
					saved_mouse_states[j].x, saved_mouse_states[j].y,
					mouse_clear.width, mouse_clear.height
				)) r_stack_push(board_base_surfs[i][0]);
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
			break;

		case CARD_MOVING:
			break;

		case TURN_END:
			break;
	}
}
