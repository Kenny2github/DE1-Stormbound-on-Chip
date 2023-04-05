#include <string.h>
#include "assets.h"
#include "image_data.h"
#include "mouse.h"
#include "game.h"
#include "states.h"
#include "vga.h"

int game_state, turn_state, move_state, player_state;

struct card cards[30] = {
	{"Lawless Herd", NEUTRAL, &lawless_herd},
	{"Felflares", NEUTRAL, &felflares},
	{"Heroic Soldiers", NEUTRAL, &heroic_soldiers},
	{"Victors of the Melee", NEUTRAL, &victors_of_the_melee},
	{"Emerald Towers", NEUTRAL, &emerald_towers},
	{"Summon Militia", NEUTRAL, &summon_militia},
	{"Execution", NEUTRAL, &execution},
	{"Blade Storm", NEUTRAL, &blade_storm},
	{"Dangeous Suitors", NEUTRAL, &dangerous_suitors},
	{"Ludic Matriarchs", NEUTRAL, &ludic_matriarchs},
	{"Shady Ghoul", SWARM, &shady_ghoul},
	{"Doppelbocks", SWARM, &doppelbocks},
	{"Moonlit Aerie", SWARM, &moonlit_aerie},
	{"Head Start", SWARM, &head_start},
	{"Dark Harvest", SWARM, &dark_harvest},
	{"Frosthexers", WINTER, &frosthexers},
	{"Wisp Cloud", WINTER, &wisp_cloud},
	{"Fleshmenders", WINTER, &fleshmenders},
	{"Moment's Peace", WINTER, &moments_peace},
	{"Icicle Burst", WINTER, &icicle_burst},
	{"Operators", IRONCLAD, &operators},
	{"Dr. Mia", IRONCLAD, &dr_mia},
	{"Agents In Charge", IRONCLAD, &agents_in_charge},
	{"Mech Workshop", IRONCLAD, &mech_workshop},
	{"Upgrade Point", IRONCLAD, &upgrade_point},
	{"Copperskin Ranger", SHADOWFEN, &copperskin_ranger},
	{"Soulcrushers", SHADOWFEN, &soulcrushers},
	{"Tode the Elevated", SHADOWFEN, &tode_the_elevated},
	{"Venomfall Spire", SHADOWFEN, &venomfall_spire},
	{"Marked As Prey", SHADOWFEN, &marked_as_prey}
};

struct image* card_selection_box[5] = {
	&neutral_box,
	&swarm_box,
	&winter_box,
	&ironclad_box,
	&shadowfen_box
};

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN;
	move_state = CARD_EFFECT;
}

static void draw_intro(void) {
	// draw logo
	draw_img_map((SCREEN_W - stormbound.width) / 2, 5, stormbound);
	// write instructions
	write_string(0, 6, instructions_data);
	// draw example cards
	draw_img_map(38, 120, felflares);
	draw_img_map(148, 120, emerald_towers);
	draw_img_map(248, 120, summon_militia);
	// draw mouse
	draw_rectangle(mouse_state.x, mouse_state.y, 2, 2, WHITE);
}

void run_game() {
	switch(game_state) {
		case TITLE:
			draw_intro();
			if (mouse_state.left_clicked) {
				game_state = DECK;
				player_state = P1;
			}
			break;
		case DECK:
			switch(player_state) {
				case P1:
					fill_screen(BACKGROUND);
					clear_char_screen();
					write_string(1, 1, "Cards:");
					for (int i = 0; i < 30; ++i) {
						draw_img_map((int)(i / 10) * 80, i % 10 * 12 + 12, *card_selection_box[cards[i].type]);
						write_string((int)(i / 10) * 20 + (20 - strlen(cards[i].name)) / 2, i % 10 * 3 + 4, cards[i].name);
					}
					
			}
	}
}
