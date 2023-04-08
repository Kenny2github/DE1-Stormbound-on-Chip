#include <stdbool.h>
#include <string.h>
#include "assets.h"
#include "image_data.h"
#include "mouse.h"
#include "timer.h"
#include "game.h"
#include "states.h"
#include "vga.h"

int game_state, turn_state, move_state, player_state;

bool in_deck[30];
int deck[2][10];
int card_num;
int row, col;
int base_health[2];

struct troop* game_board[5][4];

int health_change_num;
struct health_change health_change_list[21];

int status_change_num;
struct status_change status_change_list[20];

struct card cards[33] = {
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
	{"Marked As Prey", SHADOWFEN, &marked_as_prey},
	{"Dragon", NEUTRAL, NULL},
	{"Satyr", SWARM, NULL},
	{"Construct", IRONCLAD, NULL},
};

struct image* card_selection_box[5] = {
	&neutral_box,
	&swarm_box,
	&winter_box,
	&ironclad_box,
	&shadowfen_box
};


/* manage health change list */
static void change_healths(void) {
	return;
}

/* manage status change list */
static void change_statuses(void) {
	return;
}

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN;
	move_state = CARD_EFFECT;
}

static void draw_intro(void) {
	fill_screen(BACKGROUND);
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
				card_num = 0;
				for (int i = 0; i < 30; ++i) in_deck[i] = false;
			}
			break;


		case DECK:
			fill_screen(BACKGROUND);
			clear_char_screen();
			// draw current available cards
			write_string(1, 1, "Cards:");
			for (int i = 0; i < 30; ++i) {
				if (in_deck[i]) continue;
				draw_img_map((int)(i / 10) * 80, i % 10 * 12 + 12, *card_selection_box[cards[i].faction]);
				write_string((int)(i / 10) * 20 + (20 - strlen(cards[i].name)) / 2, i % 10 * 3 + 4, cards[i].name);
			}
			// draw current cards in deck
			write_string(61, 1, (player_state == P1) ? "P1 deck:" : "P2 deck:");
			for (int i = 0; i < card_num; ++i) {
				draw_img_map(240, i * 12 + 12, *card_selection_box[cards[deck[player_state][i]].faction]);
				write_string(60 + (20 - strlen(cards[deck[player_state][i]].name)) / 2, i * 3 + 4, cards[deck[player_state][i]].name);
			}
			// draw button for if 
			if (card_num == 10) draw_img_map(SCREEN_W - 61, 156, cardbuilding_done);
			// draw mouse
			draw_rectangle(mouse_state.x, mouse_state.y, 2, 2, WHITE);


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
				// highlight cards
				if (mouse_state.x >= 0 && mouse_state.x < 240
				&& mouse_state.y >= 12 && mouse_state.y < 132) {

					int idx = (int)(mouse_state.x / 80) * 10 + (int)((mouse_state.y - 12) / 12);
					if (!(in_deck[idx])) {
						draw_img_map(20, 156, *cards[idx].img);
					}
					

				} else if (mouse_state.x >= 240 && mouse_state.x < SCREEN_W
				&& mouse_state.y >= 12 && mouse_state.y < (card_num * 12 + 12)) {

					int idx = (mouse_state.y - 12) / 12;
					draw_img_map(20, 156, *cards[deck[player_state][idx]].img);
					
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
						for (int i = 0; i < 5; ++i) {
							for (int j = 0; j < 4; ++j) {
								game_board[i][j] = NULL;
				}
			} 
						enable_timer_interrupt();
				}
			} 

			break;

		case TURN:
			fill_screen(BACKGROUND);
			clear_char_screen();
			draw_img_map(0, 0, board);
	}
}
