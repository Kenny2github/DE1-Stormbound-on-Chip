#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "address_map_arm.h"
#include "assets.h"
#include "events.h"
#include "image_data.h"
#include "mouse.h"
#include "timer.h"
#include "game.h"
#include "render.h"
#include "states.h"
#include "timer.h"
#include "vga.h"
#include "card_logic.h"
#include "health_status.h"

#define NUM_CARDS 30

int game_state, turn_state, move_state, player_state;
int cur_round;
int time_left;

bool in_deck[30];
int deck[2][10];
int card_num;
bool cur_cards_played[4];
int cur_card_selected;

int mana;

struct troop* game_board[5][4];
int row, col;
int front[2];

int rand_num(int a, int b) {
	return rand() % (b - a + 1) + a;
}

void swap_int(int* a, int* b) {
	*a = *a + *b;
	*b = *a - *b;
	*a = *a - *b;
}

/* change mana to new_mana, and display mana value on HEX3-0 */
static void update_mana(int new_mana) {
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

static void update_front(void) {
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

/**** Static globals ****/

static const struct surface intro_surfs[] = {
	{64, 5, &stormbound},
	{38, 120, &felflares},
	{148, 120, &emerald_towers},
	{248, 120, &summon_militia},
};
#define NUM_INTRO_SURFS (sizeof(intro_surfs) / sizeof(intro_surfs[0]))

static struct surface deckbuilding_card_surfs[NUM_CARDS];
static struct surface deckbuilding_deck_surfs[10];
static bool deckbuilding_displaying;
static int deckbuilding_cur_display;
static bool deckbuilding_done_displaying;

/**** Static functions ****/

static void default_event_handlers(struct event_t event) {
	volatile int* LEDR_ptr = (int*) LEDR_BASE;
	switch (event.type) {
	case E_MOUSE_ENABLED:
		printf("Mouse plugged in\n");
		break;
	case E_MOUSE_BUTTON_DOWN:
		if (event.data.mouse_button_down.left) printf("Left button pressed\n");
		break;
	case E_MOUSE_BUTTON_UP:
		if (event.data.mouse_button_up.left) printf("Left button released\n");
		break;
	case E_MOUSE_MOVE:
		break;
	case E_TIMER_ENABLE:
		time_left = 10;
		*LEDR_ptr = (1 << time_left) - 1;
		break;
	case E_TIMER_RELOAD:
		if(!(--time_left)) disable_timer();
		*LEDR_ptr = (1 << time_left) - 1;
		break;
	case E_INTVAL_TIMER_ENABLE:
		animation_waiting = true;
		break;
	case E_INTVAL_TIMER_RELOAD:
		animation_waiting = false;
		break;
	default: ;
	}
}

static void draw_intro(void) {
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
static void run_title(void) {
	bool mouse_moved = false;
	while (!event_queue_empty()) {
		struct event_t event = event_queue_pop();
		default_event_handlers(event);

		if (event.type == E_MOUSE_BUTTON_DOWN && event.data.mouse_button_down.left) {
			// state transition on mouse click
			game_state = DECK;
			player_state = P1;
			card_num = 0;
			for (int i = 0; i < sizeof(in_deck) / sizeof(in_deck[0]); ++i) {
				in_deck[i] = false;
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

static void draw_deckbuilding(void) {
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

static void run_deckbuilding(void) {
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

/**** Exported functions ****/

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN_BUILDING;
	move_state = CARD_EFFECT;
	draw_intro();
	render_stack();
}

void run_game() {
	switch(game_state) {
		case TITLE:
			run_title();
			break;


		case DECK:
			fill_screen(BACKGROUND);
			clear_char_screen();
			// draw current available cards
			write_string(1, 1, "Cards:");
			for (int i = 0; i < 30; ++i) {
				if (in_deck[i]) continue;
				push_image((int)(i / 10) * 80, i % 10 * 12 + 12, card_selection_box[card_data[i].faction]);
				write_string((int)(i / 10) * 20 + (20 - strlen(card_data[i].name)) / 2, i % 10 * 3 + 4, card_data[i].name);
			}
			// draw current cards in deck
			write_string(61, 1, (player_state == P1) ? "P1 deck:" : "P2 deck:");
			for (int i = 0; i < card_num; ++i) {
				push_image(240, i * 12 + 12, card_selection_box[card_data[deck[player_state][i]].faction]);
				write_string(60 + (20 - strlen(card_data[deck[player_state][i]].name)) / 2, i * 3 + 4, card_data[deck[player_state][i]].name);
			}
			// draw button for if
			if (card_num == 10) push_image(SCREEN_W - 61, 156, &cardbuilding_done);
			// draw mouse
			push_image(mouse_state.x, mouse_state.y, &mouse);


			if (mouse_state.left_clicked) {
				if (card_num != 10
				 && mouse_state.x >= 0 && mouse_state.x < 240
				 && mouse_state.y >= 12 && mouse_state.y < 132) {	// clicked on available cards

					int idx = (int)(mouse_state.x / 80) * 10 + (int)((mouse_state.y - 12) / 12);
					if (!(in_deck[idx])) {	// move card to deck
						in_deck[idx] = true;
						deck[player_state][card_num++] = idx;
					}

				} else if (mouse_state.x >= 240 && mouse_state.x < SCREEN_W
				 && mouse_state.y >= 12 && mouse_state.y < (card_num * 12 + 12)) {	// clicked on deck

					int idx = (mouse_state.y - 12) / 12;
					in_deck[deck[player_state][idx]] = false; // remove card from deck
					for (int i = idx; i < card_num - 1; ++i) {	// shift deck cards down
						deck[player_state][i] = deck[player_state][i + 1];
					}
					--card_num;

				}
			} else {
				// highlight card_data
				if (mouse_state.x >= 0 && mouse_state.x < 240
				 && mouse_state.y >= 12 && mouse_state.y < 132) {

					int idx = (int)(mouse_state.x / 80) * 10 + (int)((mouse_state.y - 12) / 12);
					if (!(in_deck[idx])) {
						push_image(20, 156, card_data[idx].img);
						write_string(28, 43, card_data[idx].desc);
					}


				} else if (mouse_state.x >= 240 && mouse_state.x < SCREEN_W
				 && mouse_state.y >= 12 && mouse_state.y < (card_num * 12 + 12)) {

					int idx = (mouse_state.y - 12) / 12;
					push_image(20, 156, card_data[deck[player_state][idx]].img);
					write_string(28, 43, card_data[deck[player_state][idx]].desc);

				}
			}

			/* done button */
			if (mouse_state.x >= SCREEN_W - 61 && mouse_state.x < SCREEN_W - 20
			 && mouse_state.y >= 156 && mouse_state.y < 217 && mouse_state.left_clicked) {
				switch(player_state) {
					case P1: // go to next player
						player_state = P2;
						card_num = 0;
						for (int i = 0; i < 30; ++i) in_deck[i] = false;
						break;
					case P2: // go to turn state
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
						enable_timer_interrupt();
				}
			}

			break;


		case TURN:
			fill_screen(BACKGROUND);
			clear_char_screen();
			write_string(1, 1, (player_state == P1) ? "P1 turn" : "P2 turn");
			push_image(0, 12, &board);
			switch(turn_state) {
				case PRETURN_BUILDING:
					switch (move_state) {
						case CARD_EFFECT:
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
								health_change_num = 0;
								status_change_num = 0;
								start_turn_action(row, col);
								change_healths();
								change_statuses();
							}
							break;

						case CARD_MOVE_FORWARD:
							move_state = CARD_EFFECT;
					}

					break;

				case PRETURN_UNIT:
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
									turn_state = SELECT_CARD;
									for (int i = 0; i < 4; ++i) cur_cards_played[i] = false;
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

					break;

				case SELECT_CARD:
					// draw mouse
					push_image(mouse_state.x, mouse_state.y, &mouse);
					for (int i = 0; i < 4; ++i) {
						if (!(cur_cards_played[i])) {
							push_image(i * 46 + 10, 166, card_data[deck[player_state][i]].img);
						}
					}

					if (mouse_state.left_clicked) {
						if (mouse_state.y >= 166 && mouse_state.y < 227) {
							for (int i = 0; i < 4; ++i) {
								if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
									cur_card_selected = i;
									turn_state = PLACE_CARD;
									break;
								}
							}
						}
						// TODO: END TURN BUTTON
					} else {
						if (mouse_state.y >= 166 && mouse_state.y < 227) {
							for (int i = 0; i < 4; ++i) {
								if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
									write_string(52, 43, card_data[deck[player_state][i]].desc);
									continue;
								}
							}
						}
					}

					break;

				case PLACE_CARD:
					// draw mouse
					push_image(mouse_state.x, mouse_state.y, &mouse);
					for (int i = 0; i < 4; ++i) {
						if (!(cur_cards_played[i])) {
							push_image(i * 46 + 10, 166, card_data[deck[player_state][i]].img);
						}
					}

					if (mouse_state.left_clicked) {
						if (mouse_state.y >= 166 && mouse_state.y < 227) {
							for (int i = 0; i < 4; ++i) {
								if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
									cur_card_selected = i;
									turn_state = PLACE_CARD;
									break;
								}
							}
						} else if (mouse_state.y >= 17 && mouse_state.y < 127) {
							for (col = 0; col < 5 && ((player_state == P1 && col <= front[P1])
							 || (player_state == P2 && col >= front[P2])) && turn_state == PLACE_CARD; ++col) {
								for (row = 0; row < 4; ++row) {
									if (mouse_state.x >= col*42+56 && mouse_state.x < col*42+106
									 && mouse_state.y >= row*28+17 && mouse_state.x < row*28+43) {
										if (valid_play_card()) {
											turn_state = CARD_MOVING;
											move_state = CARD_EFFECT;
											update_mana(mana - card_data[deck[player_state][cur_card_selected]].cost);
											break;
										};
									}
								}
							}
						}
						// TODO: END TURN BUTTON
					} else {
						if (mouse_state.y >= 166 && mouse_state.y < 227) {
							for (int i = 0; i < 4; ++i) {
								if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
									write_string(52, 43, card_data[deck[player_state][i]].desc);
									continue;
								}
							}
						}
					}

					break;

				case CARD_MOVING:
					switch(move_state) {
						case CARD_EFFECT:
							play_card();
							move_state = CARD_MOVE_FORWARD;
							break;
						case CARD_MOVE_FORWARD:
							;
					}

				case TURN_END:
					;
			}
	}
}
