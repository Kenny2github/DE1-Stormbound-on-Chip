#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "assets.h"
#include "image_data.h"
#include "mouse.h"
#include "timer.h"
#include "game.h"
#include "states.h"
#include "vga.h"

#define VOTM_DMG -2
#define ET_HEAL 2
#define MA_HEAL 1
#define WC_DMG -4
#define MW_SPAWN 1
#define UP_HEAL 1

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

/* basic func for returning random number between a and b (inclusive)*/
static int rand_num(int a, int b) {
	return rand() % (b - a + 1) + a;
}

/* troop action at start of turn */
void card_action(void) {
	switch (game_board[col][row]->card_id) {
		case VICTORS_OF_THE_MELEE:
			if ((col == 4 && player_state == P1) || (col == 0 && player_state == P2)) {
				health_change_list[++health_change_num] = (struct health_change){
					0, 
					(player_state == P1 ? 5 : -1), 
					VOTM_DMG, 
					0
				};
			}
			for (int i = col - 1; i <= col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = row - 1; j <= row + 1; ++j) {
					if (j < 0 || j > 3 || (i == col && j == row)) continue;
					if (game_board[i][j] != NULL && game_board[i][j]->player != player_state) {
						health_change_list[++health_change_num] = (struct health_change){
							j,
							i, 
							VOTM_DMG, 
							0
						};
					}
				}	
			}
			break;
		case EMERALD_TOWERS:
			for (int i = 1; i < 5-col; ++i) {
				int cur_col = col + (player_state == P1 ? i : -i);
				if (game_board[cur_col][row] != NULL
				 && game_board[cur_col][row]->player == player_state
				 && game_board[cur_col][row]->type == UNIT) {
					health_change_list[++health_change_num] = (struct health_change){
						cur_col,
						row, 
						ET_HEAL, 
						0
					};
				}
			}
			break;
		case MOONLIT_AERIE:
			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < 4; ++j) {
					if (game_board[i][j] != NULL
					 && game_board[i][j]->player == player_state
					 && (game_board[i][j]->card_id == LAWLESS_HERD
					  || game_board[i][j]->card_id == SHADY_GHOUL
					  || game_board[i][j]->card_id == DOPPELBOCKS
					  || game_board[i][j]->card_id == SATYR)) {
						health_change_list[++health_change_num] = (struct health_change){
							j,
							i, 
							MA_HEAL, 
							0
						};
					}
				}	
			}
			break;
		case WISP_CLOUD:
			for (int i = col - 1; i <= col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = row - 1; j <= row + 1; ++j) {
					if (j < 0 || j > 3 || (i == col && j == row)) continue;
					if (game_board[i][j] != NULL && game_board[i][j]->status == FROZEN) {
						health_change_list[++health_change_num] = (struct health_change){
							j,
							i, 
							WC_DMG, 
							0
						};
					}
				}	
			}
			break;
		case MECH_WORKSHOP: ;
			int spawn_col = col + (player_state == P1 ? 1 : -1);
			if (spawn_col >= 0 && spawn_col <= 4 && game_board[spawn_col][row] == NULL) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					spawn_col, 
					MW_SPAWN, 
					CONSTRUCT
				};
			}
			break;
		case UPGRADE_POINT:
			for (int i = col - 1; i <= col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = row - 1; j <= row + 1; ++j) {
					if (j < 0 || j > 3 || (i == col && j == row)) continue;
					if (game_board[i][j] != NULL 
					 && game_board[i][j]->player == player_state
					 && (game_board[i][j]->card_id == OPERATORS
					  || game_board[i][j]->card_id == CONSTRUCT)) {
						health_change_list[++health_change_num] = (struct health_change){
							j,
							i, 
							UP_HEAL, 
							0
						};
					}
				}	
			}
			break;
		case SOULCRUSHERS: ;
			int destroy_col = col + (player_state == P1 ? 1 : -1);
			if (destroy_col >= 0
			 && destroy_col <= 4
			 && game_board[destroy_col][row] != NULL
			 && game_board[destroy_col][row]->player != player_state
			 && game_board[destroy_col][row]->type == UNIT
			 && game_board[destroy_col][row]->health < game_board[col][row]->health) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					destroy_col, 
					-game_board[destroy_col][row]->health, 
					0
				};
			}
			break;
		case VENOMFALL_SPIRE: ;
			int cur_rows[20], cur_cols[20], list_num = 0;
			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < 4; ++j) {
					if (game_board[i][j] != NULL
					 && game_board[i][j]->player != player_state
					 && game_board[i][j]->type == UNIT) {
						cur_rows[list_num] = j;
						cur_cols[list_num++] = i;
					}
				}	
			}
			if (list_num > 0) {
				int idx = rand_num(0, list_num - 1);
				status_change_list[++status_change_num] = (struct status_change){
					cur_rows[idx],
					cur_cols[idx],
					POISONED
				};
			}
			break;
		default:
			;
	}

	if (health_change_num != 0) move_state = CARD_EFFECT;
}

/* default unit action at start of turn */
static void move_forward(void) {
	if ((col == 4 && player_state == P1) || (col == 0 && player_state == P2)) {
		base_health[player_state] = base_health[player_state] <= game_board[col][row]->health ?
			0 : base_health[player_state] - game_board[col][row]->health;
		free(game_board[col][row]);
		game_board[col][row] = NULL;
	} else {
		if (game_board[col + 1 - player_state * 2][row] != NULL) {
			if (game_board[col + 1 - player_state * 2][row]->health <= game_board[col][row]->health) {
				if ((game_board[col][row]->health -= game_board[col + 1 - player_state * 2][row]->health) <= 0) {
					free(game_board[col][row]);
					game_board[col][row] = NULL;
				}
				free(game_board[col + 1 - player_state * 2][row]);
				game_board[col + 1 - player_state * 2][row] = NULL;
			} else {
				game_board[col + 1 - player_state * 2][row]->health -= game_board[col][row]->health;
				free(game_board[col][row]);
				game_board[col][row] = NULL;
			}
		}
	}
}

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
			write_string(1, 1, (player_state == P1) ? "P1 turn" : "P2 turn");
			draw_img_map(0, 12, board);
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
								card_action();
								change_healths();
								change_statuses();
							}
							break;

						case CARD_MOVE_FORWARD:
							move_state = CARD_EFFECT;
							break;
					}
					
				
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
									break;
								}
							}
							if (turn_state == PRETURN_UNIT) {
								health_change_num = 0;
								status_change_num = 0;
								card_action();
								change_healths();
								change_statuses();
							}
							break;

						case CARD_MOVE_FORWARD:
							move_forward();
							break;
					}
			}
	}
}