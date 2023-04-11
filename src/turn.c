#include <stdlib.h>
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

#define CARD_DESC_POS 52, 43

bool cur_cards_played[4];
int cur_card_selected;
int cur_card_displayed;
bool cur_card_displaying;

int moves_left;
int next_mov_row, next_mov_col;
bool await_next_move;

int tte_row, tte_col;
bool move_waived[COL][ROW];

int game_end_animation_timer;

struct surface tile_base_surfs[COL][ROW];
struct surface tile_overlay_surfs[COL][ROW][4];
int tile_overlay_surf_num[COL][ROW];
struct surface board_base_surfs[5];
struct surface cur_card_deck_surfs[4];
struct surface new_turn_surf = {200, 207, &end_turn};

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

void update_front(int player) {
	int cur_front;

	for (int i = COL-1; i > 0; --i) {
		cur_front = player == P1 ? i : COL-1-i;
		for (int j = 0; j < ROW; ++j) {
			if (game_board[cur_front][j] != NULL
			 && game_board[cur_front][j]->player == player) {
				if (player == P1 && cur_front == 4) cur_front = 3;
				else if (player == P2 && cur_front == 0) cur_front = 1;
				front_columns[player] = cur_front;
				return;
			}
		}
	}
	front_columns[player] = player == P1 ? 0 : 4;
}

void redraw_fronts(void) {
	int prev_front[] = {front_columns[P1], front_columns[P2]};
	update_front(P1);
	update_front(P2);
	if (front_columns[P1] != prev_front[P1] || front_columns[P2] != prev_front[P2]) {
		r_stack_push(board_base_surfs[TILES]);
		board_base_surfs[FRONT_P1].x = (front_columns[P1] + 1) * 42 + 54;
		r_stack_push(board_base_surfs[FRONT_P1]);
		board_base_surfs[FRONT_P2].x = (front_columns[P2]) * 42 + 55;
		r_stack_push(board_base_surfs[FRONT_P2]);
	}
}

static void t_display_card_info(int card_id) {
	if (card_id != cur_card_displayed) {
		write_string(CARD_DESC_POS, empty_desc_data);
		write_string(CARD_DESC_POS, card_data[card_id].desc);
	} else if (!cur_card_displaying) {
		write_string(CARD_DESC_POS, card_data[card_id].desc);
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
	front_columns[P1] = 0;
	front_columns[P2] = 4;
	base_health[P1] = 10;
	base_health[P2] = 10;
	display_base_health();
	// shuffle both decks
	for (int i = 0; i < 10; ++i) {
		swap_int(&deck[P1][i], &deck[P1][rand_num(0, 9)]);
		swap_int(&deck[P2][i], &deck[P2][rand_num(0, 9)]);
	}
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 4; ++j) {
			game_board[i][j] = NULL;
			move_waived[i][j] = false;
		}
	}
	cur_round = 0;
	update_mana(3);
	reset_health_status_changes();
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
	r_stack_push(new_turn_surf);
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
	turn_state = CARD_MOVING;
	move_state = CARD_FIND_MOVE;
	reset_health_status_changes();
	play_card();
	moves_left = card_data[deck[player_state][cur_card_selected]].init_move;

	update_mana(mana - card_data[deck[player_state][cur_card_selected]].cost);

	cur_cards_played[cur_card_selected] = true;
	push_image(cur_card_deck_surfs[cur_card_selected].x, cur_card_deck_surfs[cur_card_selected].y, &clear_card);
	swap_int(&deck[player_state][cur_card_selected], &deck[player_state][4]);
	for (int i = 4; i < 9; ++i) swap_int(&deck[player_state][i], &deck[player_state][i+1]);
	cur_card_displaying = false;
	write_string(CARD_DESC_POS, empty_desc_data);
}

static void init_game_end() {
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 4; ++j) {
			free(game_board[i][j]);
			game_board[i][j] = NULL;
		}
	}
	fill_screen(BACKGROUND);
	clear_char_screen();
	if (base_health[P1] == 0) write_string(37, 30, "P2 Win");
	else write_string(37, 30, "P1 Win");
	game_end_animation_timer = 10;
	turn_state = GAME_END;
	enable_intval_timer_interrupt();
}

static void run_preturn_building(void) {
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);
	}
	if (animation_waiting) return;

	switch (move_state) {
		case CARD_EFFECT:
			if (health_change_num == 0 && status_change_num == 0) {
				while (col == -1 || col == 5 || game_board[col][row] == NULL || game_board[col][row]->player != player_state || game_board[col][row]->type != BUILDING) {
					if ((player_state == P1) ? (++row == 4) : (--row == -1)) {
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
				reset_health_status_changes();
				if (turn_state == PRETURN_BUILDING) {
					start_turn_action(row, col, row, col);
					if (health_change_num == 0 && status_change_num == 0) move_state = CARD_MOVE;
				}
			}
			if (turn_state == PRETURN_BUILDING) {
				rerender_affected_tile();
				if (status_change_num == 0) {
					if (health_change_num == 0) {
						move_state = CARD_MOVE;
					} else {
						change_healths();
						redraw_fronts();
						if (health_change_num == 0 && status_change_num == 0) move_state = CARD_MOVE;
					}
				} else {
					change_statuses();
					if (health_change_num == 0 && status_change_num == 0) move_state = CARD_MOVE;
				}
				break;
			}
			break;

		case CARD_MOVE:
			move_state = CARD_EFFECT;
			if ((player_state == P1) ? (++row == 4) : (--row == -1)) {
				if (player_state == P1) {
					--col;
					row = 0;
				} else {
					++col;
					row = 3;
				}
			}
	}
}

static void run_preturn_unit(void) {
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);
	}
	if (animation_waiting) return;

	switch (move_state) {
		case CARD_EFFECT:
			if (health_change_num == 0 && status_change_num == 0) {
				while (col == -1 || col == 5 || game_board[col][row] == NULL || game_board[col][row]->player != player_state || game_board[col][row]->type != UNIT) {
					if ((player_state == P1) ? (++row == 4) : (--row == -1)) {
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
				reset_health_status_changes();
				if (turn_state == PRETURN_UNIT && !game_board[col][row]->frozen) {
					start_turn_action(row, col, row, col+1-player_state*2);
					next_mov_row = row;
					next_mov_col = col+1-player_state*2;
				}
			}
			if (turn_state == PRETURN_UNIT) {
				rerender_affected_tile();
				if (status_change_num == 0) {
					if (health_change_num == 0) {
						move_state = CARD_MOVE;
					} else {
						change_healths();
						redraw_fronts();
						if (health_change_num == 0 && status_change_num == 0) {
							move_state = CARD_MOVE;
							next_mov_row = row;
							next_mov_col = col+1-player_state*2;
						}
					}
				} else {
					change_statuses();
					if (health_change_num == 0 && status_change_num == 0) {
						move_state = CARD_MOVE;
						next_mov_row = row;
						next_mov_col = col+1-player_state*2;
					}
				}
				break;
			}
			break;

		case CARD_MOVE:
			if (game_board[col][row] != NULL) {
				if (game_board[col][row]->frozen) {
					push_status_change(row, col, CLEAR_FROZEN);
					change_statuses();
					status_change_num = 0;
				} else {
					if (!move_waived[col][row]
					&& move_to_tile(&row, &col, next_mov_row, next_mov_col)
					&& game_board[next_mov_col][next_mov_row] != NULL
					&& game_board[next_mov_col][next_mov_row]->card_id == TODE_THE_ELEVATED
					&& game_board[next_mov_col][next_mov_row]->player == player_state) {
						find_tode_the_elevated_jump(next_mov_row, next_mov_col, &tte_row, &tte_col);
						move_to_tile(&next_mov_row, &next_mov_col, tte_row, tte_col);
						move_waived[tte_col][tte_row] = true;
					}
				}
			}
			move_state = CARD_EFFECT;
			if (health_change_num == 0 && status_change_num == 0) {
				if ((player_state == P1) ? (++row == 4) : (--row == -1)) {
					if (player_state == P1) {
						--col;
						row = 0;
					} else {
						++col;
						row = 3;
					}
				}
			}
			redraw_fronts();
			enable_intval_timer_interrupt();
	}
}

// called on click
static void select_card_click(void) {
	if (saved_mouse_states[0].y >= 166 && saved_mouse_states[0].y < 227) {
		for (int i = 0; i < 4; ++i) {
			if (saved_mouse_states[0].x >= i * 46 + 10 && saved_mouse_states[0].x < i * 46 + 51) {
				if (!cur_cards_played[i] && i != cur_card_selected) {
					if (!cur_cards_played[cur_card_selected]
					 && cur_card_selected >= 0 && cur_card_selected < 4) {
						// clear card image
						push_image(cur_card_selected * 46 + 10, 146, &clear_card);
						// redraw but slightly lower
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
	if (rects_collide(
		new_turn_surf.x, new_turn_surf.y,
		new_turn_surf.data->width, new_turn_surf.data->height,
		saved_mouse_states[0].x, saved_mouse_states[0].y,
		mouse_clear.width, mouse_clear.height
	)) turn_state = TURN_END;
}

static void select_card_hover(void) {
	for (int i = 0; i < 4; ++i) {
		if (!cur_cards_played[i] && rects_collide(
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
		write_string(CARD_DESC_POS, empty_desc_data);
	}
}

static void select_card_rerendering(void) {
	// re-render affected surfaces
	for (int i = 0; i < 4; ++i) {	// cards
		for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
			if (!cur_cards_played[i] && rects_collide(
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
	for (int j = 1; j < NUM_MOUSE_STATES; ++j) {
		if (rects_collide(
			// end turn button
			new_turn_surf.x, new_turn_surf.y,
			new_turn_surf.data->width, new_turn_surf.data->height,
			// mouse
			saved_mouse_states[j].x, saved_mouse_states[j].y,
			mouse_clear.width, mouse_clear.height
		)) r_stack_push(new_turn_surf);
	}
}

static void run_select_card(void) {
	if (base_health[P1] == 0 || base_health[P2] == 0) {
		init_game_end();
		return;
	}
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_TIMER_RELOAD && time_left == 0) {
			turn_state = TURN_END;

		} else if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
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

		if (event.type == E_TIMER_RELOAD && time_left == 0) {
			turn_state = TURN_END;

		} else if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
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
				if (turn_state != PLACE_CARD) break;
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
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);
	}

	if (animation_waiting) return;

	switch (move_state) {
		case CARD_FIND_MOVE:
			if (health_change_num != 0 || status_change_num != 0) {
				move_state = CARD_EFFECT;
				await_next_move = false;
			} else if (game_board[col][row] != NULL && moves_left-- != 0) {
				find_next_move(row, col, &next_mov_row, &next_mov_col);
				await_next_move = true;
				move_state = CARD_EFFECT;
				enable_intval_timer_interrupt();
			} else {
				turn_state = SELECT_CARD;
			}
			break;

		case CARD_EFFECT:
			rerender_affected_tile();
			if (status_change_num == 0) {
				if (health_change_num == 0) {
					if (await_next_move) move_state = CARD_MOVE;
					else move_state = CARD_FIND_MOVE;
				} else {
					change_healths();
					redraw_fronts();
				}
			} else {
				change_statuses();
			}
			break;

		case CARD_MOVE:
			if (move_to_tile(&row, &col, next_mov_row, next_mov_col)
			 && game_board[col][row] != NULL
			 && game_board[col][row]->card_id == TODE_THE_ELEVATED) {
				find_tode_the_elevated_jump(row, col, &next_mov_row, &next_mov_col);
			} else {
				move_state = CARD_FIND_MOVE;
			}
			redraw_fronts();
			await_next_move = false;
			enable_intval_timer_interrupt();
			break;

	}
}

static void run_turn_end(void) {
	disable_timer();
	for (int i = 0; i < 4; ++i) push_image(i * 46 + 10, 146, &clear_card);
	if (player_state == P1) ++cur_round;
	player_state = player_state == P1 ? P2 : P1;
	write_string(1, 1, player_state == P1 ? "P1 turn" : "P2 turn");
	update_mana(cur_round + 3);
	init_select_card();
	reset_health_status_changes();
	turn_state = PRETURN_BUILDING;
	move_state = CARD_EFFECT;
	row = (player_state == P1) ? 0 : 3;
	col = (player_state == P1) ? 4 : 0;
	for (int i = 0; i < COL; ++i) {
		for (int j = 0; j < ROW; ++j) {
			move_waived[i][j] = false;
		}
	}
	enable_timer_interrupt();
}

static void run_game_end(void) {
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);
	}
	if (animation_waiting) return;
	if (--game_end_animation_timer == 0) {
		init_game();
	} else {
		enable_intval_timer_interrupt();
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
			run_turn_end();
			break;

		case GAME_END:
			run_game_end();
			break;
	}
}
