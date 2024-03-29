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
int front_columns[2];

/**** Exported functions ****/

int rand_num(int a, int b) {
	return rand() % (b - a + 1) + a;
}

void swap_int(int* a, int* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
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
	case E_MOUSE_BUTTON_UP:
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
		disable_intval_timer();
		break;
	default: ;
	}
}

void init_game() {
	volatile int* LEDR_ptr = (int*) LEDR_BASE;
	*LEDR_ptr = 0;
	volatile int* HEX30_ptr = (int*)HEX3_HEX0_BASE;
	*HEX30_ptr = 0;
	game_state = TITLE;
	turn_state = PRETURN_BUILDING;
	move_state = CARD_EFFECT;
	animation_waiting = false;
	draw_intro();
	render_stack();
	srand(time(NULL));
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
	}
}
