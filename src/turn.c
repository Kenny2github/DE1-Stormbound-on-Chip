#include "turn.h"
#include "address_map_arm.h"
#include "vga.h"
#include "render.h"
#include "mouse.h"
#include "states.h"
#include "events.h"
#include "event_types.h"
#include "game.h"
#include "health_status.h"

#define NUM_BOARD_BASE_SURFS 23
struct surface board_base_surfs[NUM_BOARD_BASE_SURFS];

struct surface board_overlay_surfs[22][4];

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
	enable_timer_interrupt();
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 5; ++j) {
			board_base_surfs[j+i*5] = (struct surface){i*42+56, j*28+17, &empty_tile};
			r_stack_push(board_base_surfs[j+i*5]);
		}
	}
	board_base_surfs[20] = (struct surface){0, 12, &board_p1};
	board_base_surfs[21] = (struct surface){55, 12, &board_tiles};
	board_base_surfs[21] = (struct surface){265, 12, &board_p2};
	for (int i = 0; i < 3; ++i) r_stack_push(board_base_surfs[i+20]);
}
