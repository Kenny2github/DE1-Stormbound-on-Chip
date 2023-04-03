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

void run_game() {
	if (game_state == TITLE) {
		fill_screen(BACKGROUND);
		draw_img_map((SCREEN_W - stormbound.width) / 2, 5, stormbound);
		draw_img_map(10, SCREEN_H - 10 - 60, heroic_soldiers);
		if (mouse_state.left_clicked) {
			game_state = DECK;
			player_state = P1;
		}
	} else if (game_state == DECK) {
		fill_screen(BACKGROUND);
		clear_char_screen();
	}
}
