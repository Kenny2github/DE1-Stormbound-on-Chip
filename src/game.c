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
#include "title.h"
#include "deckbuilding.h"
#include "turn.h"

int game_state, turn_state, move_state, player_state;
int cur_round;
int time_left;

int deck[2][10];

int mana;

struct troop* game_board[5][4];
int row, col;
int front[2];

/**** Exported functions ****/

int rand_num(int a, int b) {
	return rand() % (b - a + 1) + a;
}

void swap_int(int* a, int* b) {
	*a = *a + *b;
	*b = *a - *b;
	*a = *a - *b;
}

int col2x(int c) {
	return c * 42 + 56;
}

int row2y(int r) {
	return r * 28 + 17;
}

int x2col(int x) {
	return (x - 56) / 42;
}

int y2row(int y) {
	return (y - 17) / 28;
}

void default_event_handlers(struct event_t event) {
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

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN_BUILDING;
	move_state = CARD_EFFECT;
	animation_waiting = false;
	draw_intro();
	render_stack();
}

void run_game() {
	switch(game_state) {
		case TITLE:
			run_title();
			break;


		case DECK:
			run_deckbuilding();
			break;


		case TURN:
			run_turn();

			// fill_screen(BACKGROUND);
			// clear_char_screen();
			// write_string(1, 1, (player_state == P1) ? "P1 turn" : "P2 turn");
			// push_image(0, 12, &board_p1);
			// push_image(55, 12, &board_tiles);
			// push_image(265, 12, &board_p2);
			// switch(turn_state) {
			// 	case PRETURN_BUILDING:
			// 		switch (move_state) {
			// 			case CARD_EFFECT:
			// 				if (health_change_num == 0 && status_change_num == 0) {
			// 					while (game_board[col][row] == NULL || game_board[col][row]->type != BUILDING) {
			// 						if (((player_state == P1) ? ++row : --row) == 4) {
			// 							if (player_state == P1) {
			// 								--col;
			// 								row = 0;
			// 							} else {
			// 								++col;
			// 								row = 3;
			// 							}
			// 						}
			// 						if ((col == -1 && player_state == P1) || (col == 5 && player_state == P2)) {
			// 							turn_state = PRETURN_UNIT;
			// 							row = (player_state == P1) ? 0 : 3;
			// 							col = (player_state == P1) ? 4 : 0;
			// 							break;
			// 						}
			// 					}
			// 				}
			// 				if (turn_state == PRETURN_BUILDING) {
			// 					if (health_change_num == 0 && status_change_num == 0) {
			// 						start_turn_action(row, col);
			// 						health_change_idx = 0;
			// 						status_change_idx = 0;
			// 					}
			// 					change_statuses();
			// 					if (status_change_num == 0) change_healths();
			// 				}
			// 				break;

			// 			case CARD_MOVE_FORWARD:
			// 				move_state = CARD_EFFECT;
			// 		}

			// 		break;

			// 	case PRETURN_UNIT:
			// 		switch (move_state) {
			// 			case CARD_EFFECT:
			// 				while (game_board[col][row] == NULL || game_board[col][row]->type != UNIT) {
			// 					if (((player_state == P1) ? ++row : --row) == 4) {
			// 						if (player_state == P1) {
			// 							--col;
			// 							row = 0;
			// 						} else {
			// 							++col;
			// 							row = 3;
			// 						}
			// 					}
			// 					if ((col == -1 && player_state == P1) || (col == 5 && player_state == P2)) {
			// 						turn_state = SELECT_CARD;
			// 						for (int i = 0; i < 4; ++i) cur_cards_played[i] = false;
			// 						break;
			// 					}
			// 				}
			// 				if (turn_state == PRETURN_UNIT) {
			// 					health_change_num = 0;
			// 					status_change_num = 0;
			// 					start_turn_action(row, col);
			// 					change_healths();
			// 					change_statuses();
			// 				}
			// 				break;

			// 			case CARD_MOVE_FORWARD:
			// 				move_forward();
			// 				update_front();
			// 		}

			// 		break;

			// 	case SELECT_CARD:
			// 		// draw mouse
			// 		push_image(mouse_state.x, mouse_state.y, &mouse);
			// 		for (int i = 0; i < 4; ++i) {
			// 			if (!(cur_cards_played[i])) {
			// 				push_image(i * 46 + 10, 166, card_data[deck[player_state][i]].img);
			// 			}
			// 		}

			// 		if (mouse_state.left_clicked) {
			// 			if (mouse_state.y >= 166 && mouse_state.y < 227) {
			// 				for (int i = 0; i < 4; ++i) {
			// 					if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
			// 						cur_card_selected = i;
			// 						turn_state = PLACE_CARD;
			// 						break;
			// 					}
			// 				}
			// 			}
			// 			// TODO: END TURN BUTTON
			// 		} else {
			// 			if (mouse_state.y >= 166 && mouse_state.y < 227) {
			// 				for (int i = 0; i < 4; ++i) {
			// 					if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
			// 						write_string(52, 43, card_data[deck[player_state][i]].desc);
			// 						continue;
			// 					}
			// 				}
			// 			}
			// 		}

			// 		break;

			// 	case PLACE_CARD:
			// 		// draw mouse
			// 		push_image(mouse_state.x, mouse_state.y, &mouse);
			// 		for (int i = 0; i < 4; ++i) {
			// 			if (!(cur_cards_played[i])) {
			// 				push_image(i * 46 + 10, 166, card_data[deck[player_state][i]].img);
			// 			}
			// 		}

			// 		if (mouse_state.left_clicked) {
			// 			if (mouse_state.y >= 166 && mouse_state.y < 227) {
			// 				for (int i = 0; i < 4; ++i) {
			// 					if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
			// 						cur_card_selected = i;
			// 						turn_state = PLACE_CARD;
			// 						break;
			// 					}
			// 				}
			// 			} else if (mouse_state.y >= 17 && mouse_state.y < 127) {
			// 				for (col = 0; col < 5 && ((player_state == P1 && col <= front[P1])
			// 				 || (player_state == P2 && col >= front[P2])) && turn_state == PLACE_CARD; ++col) {
			// 					for (row = 0; row < 4; ++row) {
			// 						if (mouse_state.x >= col*42+56 && mouse_state.x < col*42+106
			// 						 && mouse_state.y >= row*28+17 && mouse_state.x < row*28+43) {
			// 							if (valid_play_card()) {
			// 								turn_state = CARD_MOVING;
			// 								move_state = CARD_EFFECT;
			// 								update_mana(mana - card_data[deck[player_state][cur_card_selected]].cost);
			// 								break;
			// 							};
			// 						}
			// 					}
			// 				}
			// 			}
			// 			// TODO: END TURN BUTTON
			// 		} else {
			// 			if (mouse_state.y >= 166 && mouse_state.y < 227) {
			// 				for (int i = 0; i < 4; ++i) {
			// 					if (mouse_state.x >= i * 46 + 10 && mouse_state.x < i * 46 + 51) {
			// 						write_string(52, 43, card_data[deck[player_state][i]].desc);
			// 						continue;
			// 					}
			// 				}
			// 			}
			// 		}

			// 		break;

			// 	case CARD_MOVING:
			// 		switch(move_state) {
			// 			case CARD_EFFECT:
			// 				play_card();
			// 				move_state = CARD_MOVE_FORWARD;
			// 				break;
			// 			case CARD_MOVE_FORWARD:
			// 				;
			// 		}

			// 	case TURN_END:
			// 		;
			// }
	}
}
