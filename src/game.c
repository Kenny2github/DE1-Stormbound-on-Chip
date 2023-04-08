#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "address_map_arm.h"
#include "assets.h"
#include "image_data.h"
#include "mouse.h"
#include "timer.h"
#include "game.h"
#include "states.h"
#include "vga.h"

#define FF_DMG -2
#define VOTM_DMG -2
#define ET_HEAL 2
#define DS_HEAL 2
#define LM_SPAWN 1
#define DB_SPAWN 1
#define MA_HEAL 1
#define WC_DMG -4
#define MW_SPAWN 1
#define UP_HEAL 1
#define TTE_HEAL 2

int game_state, turn_state, move_state, player_state;
int round;

bool in_deck[30];
int deck[2][10];
int card_num;
bool cur_cards_played[4];
int cur_card_selected;

int row, col;
int base_health[2];
int mana;

struct troop* game_board[5][4];
int front[2];

int health_change_num;
struct health_change health_change_list[21];

int status_change_num;
struct status_change status_change_list[20];

int bordering_row[4] = {0, -1, 1, 0};
int bordering_col[4] = {1, 0, 0, -1};

struct card cards[35] = {
	{"Lawless Herd", NEUTRAL, &lawless_herd, lawless_herd_desc_data, 2, 2, 0},
	{"Felflares", NEUTRAL, &felflares, felflares_desc_data, 3, 2, 0},
	{"Heroic Soldiers", NEUTRAL, &heroic_soldiers, heroic_soldiers_desc_data, 5, 6, 1},
	{"Victors of the Melee", NEUTRAL, &victors_of_the_melee, victors_of_the_melee_desc_data, 6, 4, 1},
	{"Emerald Towers", NEUTRAL, &emerald_towers, emerald_towers_desc_data, 4, 4, 0},
	{"Summon Militia", NEUTRAL, &summon_militia, summon_militia_desc_data, 1, 0, 0},
	{"Execution", NEUTRAL, &execution, execution_desc_data, 4, 0, 0},
	{"Blade Storm", NEUTRAL, &blade_storm, blade_storm_desc_data, 5, 0, 0},
	{"Dangeous Suitors", NEUTRAL, &dangerous_suitors, dangerous_suitors_desc_data, 6, 4, 1},
	{"Ludic Matriarchs", NEUTRAL, &ludic_matriarchs, ludic_matriarchs_desc_data, 6, 5, 0},
	{"Shady Ghoul", SWARM, &shady_ghoul, shady_ghoul_desc_data, 3, 1, 2},
	{"Doppelbocks", SWARM, &doppelbocks, doppelbocks_desc_data, 2, 1, 0},
	{"Moonlit Aerie", SWARM, &moonlit_aerie, moonlit_aerie_desc_data, 3, 3, 0},
	{"Head Start", SWARM, &head_start, head_start_desc_data, 2, 0, 0},
	{"Dark Harvest", SWARM, &dark_harvest, dark_harvest_desc_data, 5, 0, 0},
	{"Frosthexers", WINTER, &frosthexers, frosthexers_desc_data, 2, 1, 0},
	{"Wisp Cloud", WINTER, &wisp_cloud, wisp_cloud_desc_data, 3, 1, 1},
	{"Fleshmenders", WINTER, &fleshmenders, fleshmenders_desc_data, 7, 5, 2},
	{"Moment's Peace", WINTER, &moments_peace, moments_peace_desc_data, 5, 0, 0},
	{"Icicle Burst", WINTER, &icicle_burst, icicle_burst_desc_data, 1, 0, 0},
	{"Operators", IRONCLAD, &operators, operators_desc_data, 8, 12, 1},
	{"Dr. Mia", IRONCLAD, &dr_mia, dr_mia_desc_data, 2, 2, 0},
	{"Agents in Charge", IRONCLAD, &agents_in_charge, agents_in_charge_desc_data, 5, 3, 3},
	{"Mech Workshop", IRONCLAD, &mech_workshop, mech_workshop_desc_data, 4, 3, 0},
	{"Upgrade Point", IRONCLAD, &upgrade_point, upgrade_point_desc_data, 3, 4, 0},
	{"Copperskin Ranger", SHADOWFEN, &copperskin_ranger, copperskin_ranger_desc_data, 2, 1, 0},
	{"Soulcrushers", SHADOWFEN, &soulcrushers, soulcrushers_desc_data, 5, 5, 1},
	{"Tode the Elevated", SHADOWFEN, &tode_the_elevated, tode_the_elevated_desc_data, 4, 3, 1},
	{"Venomfall Spire", SHADOWFEN, &venomfall_spire, venomfall_spire_desc_data, 4, 4, 0},
	{"Marked as Prey", SHADOWFEN, &marked_as_prey, marked_as_prey_desc_data, 4, 0, 0},
	{"Knight", NEUTRAL, NULL, knight_desc_data, 0, 0, 0},
	{"Dragon", NEUTRAL, NULL, dragon_desc_data, 0, 0, 0},
	{"Satyr", SWARM, NULL, satyr_desc_data, 0, 0, 0},
	{"Construct", IRONCLAD, NULL, construct_desc_data, 0, 0, 0},
	{"Toad", SHADOWFEN, NULL, toad_desc_data, 0, 0, 0}
};

const struct image* card_selection_box[5] = {
	&neutral_box,
	&swarm_box,
	&winter_box,
	&ironclad_box,
	&shadowfen_box
};

/* basic func for returning random number between a and b (inclusive) */
static int rand_num(int a, int b) {
	return rand() % (b - a + 1) + a;
}

/* basic func for swapping two integers */
static void swap_int(int* a, int* b) {
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

/* determine whether card at location has same type as card_id */
static bool of_same_type(int row, int col, int card_id) {
	if (row < 0 || row > 3 || col < 0 || col > 4) return false;

	struct troop* cur_troop = game_board[col][row];
	if (cur_troop == NULL) return false;

	switch(card_id) {
		case LAWLESS_HERD:
		case SHADY_GHOUL:
		case DOPPELBOCKS:
		case SATYR:
			return (cur_troop->card_id == LAWLESS_HERD
				|| cur_troop->card_id == SHADY_GHOUL
				|| cur_troop->card_id == DOPPELBOCKS
				|| cur_troop->card_id == SATYR);
			break;

		case FELFLARES:
		case FROSTHEXERS:
		case WISP_CLOUD:
			return (cur_troop->card_id == FELFLARES
				|| cur_troop->card_id == FROSTHEXERS
				|| cur_troop->card_id == WISP_CLOUD);
			break;

		case HEROIC_SOLDIERS:
		case VICTORS_OF_THE_MELEE:
		case FLESHMENDERS:
		case KNIGHT:
			return (cur_troop->card_id == HEROIC_SOLDIERS
				|| cur_troop->card_id == VICTORS_OF_THE_MELEE
				|| cur_troop->card_id == FLESHMENDERS
				|| cur_troop->card_id == KNIGHT);
			break;

		case DANGEROUS_SUITORS:
		case LUDIC_MATRIARCHS:
		case DRAGON:
			return (cur_troop->card_id == DANGEROUS_SUITORS
				|| cur_troop->card_id == LUDIC_MATRIARCHS
				|| cur_troop->card_id == DRAGON);
			break;

		case OPERATORS:
		case CONSTRUCT:
			return (cur_troop->card_id == OPERATORS
				|| cur_troop->card_id == CONSTRUCT);
			break;

		case DR_MIA:
		case AGENTS_IN_CHARGE:
		case SOULCRUSHERS:
			return (cur_troop->card_id == DR_MIA
				|| cur_troop->card_id == AGENTS_IN_CHARGE
				|| cur_troop->card_id == SOULCRUSHERS);
			break;

		case COPPERSKIN_RANGER:
		case TODE_THE_ELEVATED:
		case TOAD:
			return (cur_troop->card_id == COPPERSKIN_RANGER
				|| cur_troop->card_id == TODE_THE_ELEVATED
				|| cur_troop->card_id == TOAD);
			break;

		default:
			return (cur_troop->card_id == card_id);
	}
}

/* troop action at start of turn */
void start_turn_action(int act_row, int act_col) {
	switch (game_board[act_col][act_row]->card_id) {
		case VICTORS_OF_THE_MELEE:
			if ((act_col == 4 && player_state == P1) || (act_col == 0 && player_state == P2)) {
				health_change_list[++health_change_num] = (struct health_change){
					0, 
					(player_state == P1 ? 5 : -1), 
					VOTM_DMG, 
					0
				};
			}
			for (int i = act_col - 1; i <= act_col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = act_row - 1; j <= act_row + 1; ++j) {
					if (j < 0 || j > 3 || (i == act_col && j == act_row)) continue;
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
			for (int i = 1; i < 5-act_col; ++i) {
				int cur_col = act_col + (player_state == P1 ? i : -i);
				if (game_board[cur_col][act_row] != NULL
				 && game_board[cur_col][act_row]->player == player_state
				 && game_board[cur_col][act_row]->type == UNIT) {
					health_change_list[++health_change_num] = (struct health_change){
						cur_col,
						act_row, 
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
					 && of_same_type(j, i, SATYR)) {
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
			if (game_board[act_col+1-player_state*2][act_row]->status == FROZEN) {
				for (int i = act_col - 1; i <= act_col + 1; ++i) {
					if (i < 0 || i > 4) continue;
					for (int j = act_row - 1; j <= act_row + 1; ++j) {
						if (j < 0 || j > 3 || (i == act_col && j == act_row)) continue;
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
			}
			break;
		case MECH_WORKSHOP: ;
			int spawn_col = act_col + (player_state == P1 ? 1 : -1);
			if (spawn_col >= 0 && spawn_col <= 4 && game_board[spawn_col][act_row] == NULL) {
				health_change_list[++health_change_num] = (struct health_change){
					act_row,
					spawn_col, 
					MW_SPAWN, 
					CONSTRUCT
				};
			}
			break;
		case UPGRADE_POINT:
			for (int i = act_col - 1; i <= act_col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = act_row - 1; j <= act_row + 1; ++j) {
					if (j < 0 || j > 3 || (i == act_col && j == act_row)) continue;
					if (game_board[i][j] != NULL 
					 && game_board[i][j]->player == player_state
					 && of_same_type(j, i, CONSTRUCT)) {
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
			int destroy_col = act_col + (player_state == P1 ? 1 : -1);
			if (destroy_col >= 0
			 && destroy_col <= 4
			 && game_board[destroy_col][act_row] != NULL
			 && game_board[destroy_col][act_row]->player != player_state
			 && game_board[destroy_col][act_row]->type == UNIT
			 && game_board[destroy_col][act_row]->health < game_board[act_col][act_row]->health) {
				health_change_list[++health_change_num] = (struct health_change){
					act_row,
					destroy_col, 
					-game_board[destroy_col][act_row]->health, 
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
				if (game_board[col][row] != NULL) {
					if (game_board[col][row]->card_id == TODE_THE_ELEVATED) { // additional logic just for TTE. fuck TTE
						int cur_rows[20], cur_cols[20], list_num = 0;
						for (int i = 0; i < 5; ++i) {
							int jump_col = i - 1 + player_state*2;
							if (jump_col < 0 || jump_col > 4) continue;
							for (int j = 0; j < 4; ++j) {
								if (game_board[i][j] != NULL
								&& game_board[i][j]->player != player_state
								&& game_board[jump_col][j] == NULL) {
									cur_rows[list_num] = j;
									cur_cols[list_num++] = jump_col;
								}
							}	
						} 
						if (list_num > 0) {
							int idx = rand_num(0, list_num - 1);
							game_board[col][row]->health += TTE_HEAL;
							game_board[cur_cols[idx]][cur_rows[idx]] = game_board[col][row];
						} else {
							game_board[col+1-player_state*2][row] = game_board[col][row];
						}
					} else {
						game_board[col+1-player_state*2][row] = game_board[col][row];
					}
					game_board[col][row] = NULL;
				}
			} else {
				game_board[col + 1 - player_state * 2][row]->health -= game_board[col][row]->health;
				free(game_board[col][row]);
				game_board[col][row] = NULL;
			}
		}
	}
}

/* determine whether a card can be placed on a tile */
static bool valid_play_card(void) {
	int card_id = deck[player_state][cur_card_selected];
	if (cards[card_id].cost > mana) return false;

	switch(deck[player_state][cur_card_selected]) {
		case EXECUTION:
		case ICICLE_BURST:
		case MARKED_AS_PREY:
			return (game_board[col][row] != NULL
			 && game_board[col][row]->player != player_state);
			 break;

		case DARK_HARVEST:
		case MOMENTS_PEACE:
			return (game_board[col][row] != NULL
			 && game_board[col][row]->player == player_state);
			 break;
			
		case SUMMON_MILITIA:
		case BLADE_STORM:
		case HEAD_START:
			return true;
			break;

		default:
			return (game_board[col][row] == NULL);
	}
}

void play_card(void) {
	int card_id = deck[player_state][cur_card_selected];
	struct troop* new_troop = (struct troop*) malloc(sizeof(struct troop));
	int cur_rows[20], cur_cols[20], list_num = 0;
	/* create new troop object */
	switch(card_id) {
		case LAWLESS_HERD:
		case SHADY_GHOUL:
		case DOPPELBOCKS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &satyr_p1 : &satyr_p2
			};
			break;

		case FELFLARES:
		case FROSTHEXERS:
		case WISP_CLOUD:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &frostling_p1 : &frostling_p2
			};
			break;

		case HEROIC_SOLDIERS:
		case VICTORS_OF_THE_MELEE:
		case FLESHMENDERS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &knight_p1 : &knight_p2
			};
			break;

		case DANGEROUS_SUITORS:
		case LUDIC_MATRIARCHS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &dragon_p1 : &dragon_p2
			};
			break;

		case OPERATORS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &construct_p1 : &construct_p2
			};
			break;

		case DR_MIA:
		case AGENTS_IN_CHARGE:
		case SOULCRUSHERS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &rodent_p1 : &rodent_p2
			};
			break;

		case COPPERSKIN_RANGER:
		case TODE_THE_ELEVATED:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &toad_p1 : &toad_p2
			};
			break;
		
		case EMERALD_TOWERS:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &emerald_p1 : &emerald_p2
			};
			break;

		case MOONLIT_AERIE:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &moonlit_p1 : &moonlit_p2
			};
			break;

		case MECH_WORKSHOP:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &workshop_p1 : &workshop_p2
			};
			break;

		case UPGRADE_POINT:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &upgrade_p1 : &upgrade_p2
			};
			break;

		case VENOMFALL_SPIRE:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				cards[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &venomfall_p1 : &venomfall_p2
			};
			break;

		default:
			free(new_troop);
	}

	switch(card_id) {
		case VICTORS_OF_THE_MELEE:
		case WISP_CLOUD:
		case SOULCRUSHERS:
			// same as start turn action
			game_board[col][row] = new_troop;
			start_turn_action(row, col);
			break;

		case FELFLARES:
			game_board[col][row] = new_troop;
			for (int i = col - 1; i <= col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = row - 1; j <= row + 1; ++j) {
					if (j < 0 || j > 3 || (i == col && j == row)) continue;
					if (game_board[i][j] != NULL && game_board[i][j]->player != player_state) {
						cur_rows[list_num] = j;
						cur_cols[list_num++] = i;
					}
				}	
			}
			if (list_num > 0) {
				int idx = rand_num(0, list_num - 1);
				health_change_list[++health_change_num] = (struct health_change){
					cur_rows[idx],
					cur_cols[idx],
					FF_DMG,
					0
				};
			}
			break;

		case SUMMON_MILITIA:
			;

		case DANGEROUS_SUITORS:
			game_board[col][row] = new_troop;
			int heal = 0;
			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < 4; ++j) {
					if (game_board[i][j] != NULL
					 && of_same_type(j, i, DRAGON)) {
						++heal;
					}
				}
			}
			if (heal != 0) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					col,
					DS_HEAL * heal,
					0
				};
			}
			break;
		
		case LUDIC_MATRIARCHS:
			game_board[col][row] = new_troop;
			int adj_row = row, adj_col = col;

			// find adjacent dragon
			for (int i = 0; i < 4; ++i) {
				int check_row = row + bordering_row[player_state == P1 ? i : 3-i];
				int check_col = col + bordering_col[player_state == P1 ? i : 3-i];
				if (of_same_type(check_col, check_row, DRAGON)
				 && game_board[check_col][check_row]->player == player_state) {
					adj_row = check_row;
					adj_col = check_col;
					break;
				}
			}

			// spawn dragons on bordering tiles for both
			if (adj_row != row || adj_col != col) {
				for (int i = 0; i < 4; ++i) {
					int cur_col = col+bordering_col[i];
					int cur_row = row+bordering_row[i];
					if (game_board[cur_col][cur_row] == NULL) health_change_list[++health_change_num] = (struct health_change){
						cur_row,
						cur_col,
						LM_SPAWN,
						DRAGON
					};
					cur_col = adj_col+bordering_col[i];
					cur_row = adj_row+bordering_row[i];
					if (game_board[cur_col][cur_row] == NULL) health_change_list[++health_change_num] = (struct health_change){
						cur_row,
						cur_col,
						LM_SPAWN,
						DRAGON
					};
				}
			}
			break;
		
		case DOPPELBOCKS:
			game_board[col][row] = new_troop;
			int check_col = col+1-player_state*2;
			if (check_col >= 0 && check_col <= 4
			 && game_board[check_col][row] == NULL) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					check_col,
					DB_SPAWN,
					SATYR
				};
			}
			break;

		case FROSTHEXERS:
			game_board[col][row] = new_troop;
			for (int i = 0; i < 4; ++i) {
				int check_row = row + bordering_row[i];
				int check_col = col + bordering_col[i];
				if (game_board[check_col][check_row] != NULL
				 && game_board[check_col][check_row]->player != player_state
				 && game_board[check_col][check_row]->type == UNIT) {
					status_change_list[++status_change_num] = (struct status_change){
						check_row,
						check_col,
						FROZEN
					};
				}
			}
			break;

		case DR_MIA:
			game_board[col][row] = new_troop;
			for (int i = 0; i < 4; ++i) {
				int check_row = row + bordering_row[i];
				int check_col = col + bordering_col[i];
				if (game_board[check_col][check_row] != NULL
				 && game_board[check_col][check_row]->player == player_state
				 && game_board[check_col][check_row]->type == BUILDING) {
					// all buildings have a start of turn effect
					start_turn_action(check_row, check_col);
				}
			}
			break;

		case COPPERSKIN_RANGER:
			game_board[col][row] = new_troop;
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
						write_string(28, 43, cards[idx].desc);
					}
					

				} else if (mouse_state.x >= 240 && mouse_state.x < SCREEN_W
				 && mouse_state.y >= 12 && mouse_state.y < (card_num * 12 + 12)) {

					int idx = (mouse_state.y - 12) / 12;
					draw_img_map(20, 156, *cards[deck[player_state][idx]].img);
					write_string(28, 43, cards[deck[player_state][idx]].desc);
					
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
						round = 0;
						update_mana(3);
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
					}

					break;
				
				case SELECT_CARD:
					// draw mouse
					draw_rectangle(mouse_state.x, mouse_state.y, 2, 2, WHITE);
					for (int i = 0; i < 4; ++i) {
						if (!(cur_cards_played[i])) {
							draw_img_map(i * 46 + 10, 166, *cards[deck[player_state][i]].img);
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
									write_string(52, 43, cards[deck[player_state][i]].desc);
									continue;
								}
							}
						}
					}

					break;

				case PLACE_CARD:
					// draw mouse
					draw_rectangle(mouse_state.x, mouse_state.y, 2, 2, WHITE);
					for (int i = 0; i < 4; ++i) {
						if (!(cur_cards_played[i])) {
							draw_img_map(i * 46 + 10, 166, *cards[deck[player_state][i]].img);
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
											update_mana(mana - cards[deck[player_state][cur_card_selected]].cost);
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
									write_string(52, 43, cards[deck[player_state][i]].desc);
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