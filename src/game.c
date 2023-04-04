#include "assets.h"
#include "mouse.h"
#include "game.h"
#include "states.h"
#include "vga.h"

int game_state, turn_state, move_state, player_state;

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN;
	move_state = CARD_EFFECT;
}

static void draw_intro(void) {
	fill_screen(BACKGROUND);
	draw_img_map((SCREEN_W - stormbound.width) / 2, 5, stormbound);
	write_string(0, 6, instructions_data);
	draw_rectangle(mouse_state.x, mouse_state.y, 2, 2, WHITE);
}

void run_game() {
	if (game_state == TITLE) {
		draw_intro();
		if (mouse_state.left_clicked) {
			game_state = DECK;
			player_state = P1;
		}
	} else if (game_state == DECK) {
		fill_screen(BACKGROUND);
		clear_char_screen();
	}
}
