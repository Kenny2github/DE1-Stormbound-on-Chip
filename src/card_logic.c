#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "states.h"
#include "assets.h"
#include "card_logic.h"
#include "health_status.h"

#define FF_DMG -2
#define VOTM_DMG -2
#define ET_HEAL 2
#define SM_SPAWN 1
#define E_DMG -4
#define BS_DMG -1
#define DS_HEAL 2
#define LM_SPAWN 1
#define DB_SPAWN 1
#define MA_HEAL 1
#define HS_SPAWN 3
#define DH_DMG -2
#define WC_DMG -4
#define FM_HEAL 6
#define MP_HEAL 5
#define IB_DMG -8
#define MW_SPAWN 1
#define UP_HEAL 1
#define TTE_HEAL 2
#define MAP_DMG -5
#define MAP_SPAWN 5

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

void move_forward(void) {
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

bool valid_play_card(void) {
	int card_id = deck[player_state][cur_card_selected];
	if (card_data[card_id].cost > mana) return false;

	switch(deck[player_state][cur_card_selected]) {
		case EXECUTION:
		case ICICLE_BURST:
			return (game_board[col][row] != NULL
			 && game_board[col][row]->player != player_state);
			break;

		case MARKED_AS_PREY:
			return (game_board[col][row] != NULL
			 && game_board[col][row]->player != player_state
			 && game_board[col][row]->status == POISONED);
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
				card_data[card_id].init_health,
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
				card_data[card_id].init_health,
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
				card_data[card_id].init_health,
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
				card_data[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &dragon_p1 : &dragon_p2
			};
			break;

		case OPERATORS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				card_data[card_id].init_health,
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
				card_data[card_id].init_health,
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
				card_data[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &toad_p1 : &toad_p2
			};
			break;
		
		case EMERALD_TOWERS:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &emerald_p1 : &emerald_p2
			};
			break;

		case MOONLIT_AERIE:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &moonlit_p1 : &moonlit_p2
			};
			break;

		case MECH_WORKSHOP:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &workshop_p1 : &workshop_p2
			};
			break;

		case UPGRADE_POINT:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				NO_STATUS,
				(player_state == P1) ? &upgrade_p1 : &upgrade_p2
			};
			break;

		case VENOMFALL_SPIRE:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
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
			for (int i = 0; i <= (player_state == P1 ? front[P1] : 5-front[P2]); ++i) {
				int cur_col = player_state == P1 ? i : 5-i;
				for (int j = 0; j < 4; ++j) {
					if (game_board[cur_col][j] != NULL && game_board[cur_col][j]->player != player_state) {
						cur_rows[list_num] = j;
						cur_cols[list_num++] = cur_col;
					}
				}
			}
			if (list_num > 0) {
				int idx = rand_num(0, list_num - 1);
				health_change_list[++health_change_num] = (struct health_change){
					cur_rows[idx],
					cur_cols[idx],
					SM_SPAWN,
					KNIGHT
				};
			}
			break;

		case EXECUTION:
			health_change_list[++health_change_num] = (struct health_change){
				row,
				col,
				E_DMG,
				0
			};
			break;

		case BLADE_STORM:
			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < 4; ++j) {
					if (game_board[i][j] != NULL && game_board[i][j]->player != player_state) {
						health_change_list[++health_change_num] = (struct health_change){
							row,
							col,
							BS_DMG,
							0
						};
					}
				}
			}
			break;

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

		case HEAD_START:
			for (int i = 0; i < 4; ++i) {
				if (game_board[front[player_state]][i] == NULL) {
					cur_rows[list_num++] = i;
				}
			}
			if (list_num > 0) {
				int idx = rand_num(0, list_num - 1);
				health_change_list[++health_change_num] = (struct health_change){
					cur_rows[idx],
					front[player_state],
					HS_SPAWN,
					SATYR
				};
			}
			break;

		case DARK_HARVEST:
			for (int i = col - 1; i <= col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = row - 1; j <= row + 1; ++j) {
					if (j < 0 || j > 3 || (i == col && j == row)) continue;
					if (game_board[i][j] != NULL && game_board[i][j]->player != player_state) {
						health_change_list[++health_change_num] = (struct health_change){
							j,
							i, 
							DH_DMG, 
							0
						};
					}
				}	
			}
			break;

		case FROSTHEXERS:
			game_board[col][row] = new_troop;
			for (int i = 0; i < 4; ++i) {
				int check_row = row + bordering_row[i];
				int check_col = col + bordering_col[i];
				if (check_row < 0 || check_row > 3 || check_col < 0 || check_col > 4) continue;
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

		case FLESHMENDERS:
			game_board[col][row] = new_troop;
			for (int i = 0; i < 4; ++i) {
				int check_row = row + bordering_row[i];
				int check_col = col + bordering_col[i];
				if (check_row < 0 || check_row > 3 || check_col < 0 || check_col > 4) continue;
				if (game_board[check_col][check_row] != NULL
				 && game_board[check_col][check_row]->player == player_state) {
					cur_rows[list_num] = check_row;
					cur_cols[list_num++] = check_col;
				}
			}
			if (list_num > 0) {
				int idx = rand_num(0, list_num - 1);
				health_change_list[++health_change_num] = (struct health_change){
					cur_rows[idx],
					cur_cols[idx],
					FM_HEAL,
					0
				};
			}
			break;

		case MOMENTS_PEACE:
			health_change_list[++health_change_num] = (struct health_change){
				row,
				col,
				MP_HEAL,
				0
			};
			for (int i = col - 1; i <= col + 1; ++i) {
				if (i < 0 || i > 4) continue;
				for (int j = row - 1; j <= row + 1; ++j) {
					if (j < 0 || j > 3 || (i == col && j == row)) continue;
					if (game_board[i][j] != NULL
					 && game_board[i][j]->player != player_state
					 && game_board[i][j]->type == UNIT) {
						status_change_list[++status_change_num] = (struct status_change){
							j,
							i, 
							FROZEN
						};
					}
				}	
			}
			break;

		case ICICLE_BURST:
			if (game_board[col][row]->status == NO_STATUS) {
				status_change_list[++status_change_num] = (struct status_change){
					row,
					col,
					FROZEN
				};
			} else {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					col,
					IB_DMG,
					0
				};
			}
			break;

		case DR_MIA:
			game_board[col][row] = new_troop;
			for (int i = 0; i < 4; ++i) {
				int check_row = row + bordering_row[i];
				int check_col = col + bordering_col[i];
				if (check_row < 0 || check_row > 3 || check_col < 0 || check_col > 4) continue;
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

		case MARKED_AS_PREY:
			health_change_list[++health_change_num] = (struct health_change){
				row,
				col,
				MAP_DMG,
				0
			};
			if (game_board[col][row]->health + MAP_DMG <= 0) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					col,
					MAP_SPAWN,
					TOAD
				};
			}
			break;

		default:
			game_board[col][row] = new_troop;
	}
}