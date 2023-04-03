#include "assets.h"
#include "game.h"
#include "states.h"
#include "vga.h"

int game_state, turn_state, move_state, player_state;

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN;
	move_state = CARD_EFFECT;
	player_state = P1;
}

void run_game() {
	if (game_state == TITLE) {
		fill_screen(BACKGROUND);
		draw_img_map(0, 100, stormbound);
		write_string(0, 0, navy_seals_data);
	}
}
