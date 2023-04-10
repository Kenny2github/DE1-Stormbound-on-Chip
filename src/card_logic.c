#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "states.h"
#include "assets.h"
#include "render.h"
#include "timer.h"
#include "card_logic.h"
#include "health_status.h"
#include "turn.h"

#define FELFLARES_DAMAGE -2
#define VICTORS_OF_THE_MELEE_DAMAGE -2
#define EMERALD_TOWERS_HEAL 2
#define SUMMON_MILITIA_SPAWN 1
#define EXECUTION_DAMAGE -4
#define BLADESTORM_DAMAGE -1
#define DANGEROUS_SUITORS_HEAL 2
#define LUDIC_MATRIARCHEAD_START_SPAWN 1
#define DOPPELBOCKS_SPAWN 1
#define MOONLIT_AERIE_HEAL 1
#define HEAD_START_SPAWN 3
#define DARK_HARVEST_DAMAGE -2
#define WISP_CLOUD_DAMAGE -4
#define FLESHMENDERS_HEAL 6
#define MOMENTS_PEACE_HEAL 5
#define ICICLE_BURST_DAMAGE -8
#define MECH_WORKSHOP_SPAWN 1
#define UPGRADE_POINT_HEAL 1
#define TODE_THE_ELEVATED_HEAL 2
#define MARKED_AS_PREY_DAMAGE -5
#define MARKED_AS_PREY_SPAWN 5

void place_new_tile_asset(int r, int c, struct troop* new_troop) {
	game_board[c][r] = new_troop;
	tile_base_surfs[c][r] = (struct surface){
		col2x(c),
		row2y(r),
		new_troop->img
	};
	r_stack_push(tile_base_surfs[c][r]);
}

void remove_tile_asset(int r, int c) {
	free(game_board[c][r]);
	tile_base_surfs[c][r] = (struct surface){
		col2x(c),
		row2y(r),
		&empty_tile
	};
	r_stack_push(tile_base_surfs[c][r]);
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

void start_turn_action(int act_row, int act_col) {
	switch (game_board[act_col][act_row]->card_id) {
		case VICTORS_OF_THE_MELEE:
			if ((act_col == 4 && player_state == P1) || (act_col == 0 && player_state == P2)) {
				health_change_list[++health_change_num] = (struct health_change){
					0,
					(player_state == P1 ? 5 : -1),
					VICTORS_OF_THE_MELEE_DAMAGE,
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
							VICTORS_OF_THE_MELEE_DAMAGE,
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
						EMERALD_TOWERS_HEAL,
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
							MOONLIT_AERIE_HEAL,
							0
						};
					}
				}
			}
			break;
		case WISP_CLOUD:
			if (game_board[act_col+1-player_state*2][act_row]->frozen) {
				for (int i = act_col - 1; i <= act_col + 1; ++i) {
					if (i < 0 || i > 4) continue;
					for (int j = act_row - 1; j <= act_row + 1; ++j) {
						if (j < 0 || j > 3 || (i == act_col && j == act_row)) continue;
						if (game_board[i][j] != NULL && game_board[i][j]->frozen) {
							health_change_list[++health_change_num] = (struct health_change){
								j,
								i,
								WISP_CLOUD_DAMAGE,
								0
							};
						}
					}
				}
			}
			break;
		case MECH_WORKSHOP:
			int spawn_col = act_col + (player_state == P1 ? 1 : -1);
			if (spawn_col >= 0 && spawn_col <= 4 && game_board[spawn_col][act_row] == NULL) {
				health_change_list[++health_change_num] = (struct health_change){
					act_row,
					spawn_col,
					MECH_WORKSHOP_SPAWN,
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
							UPGRADE_POINT_HEAL,
							0
						};
					}
				}
			}
			break;
		case SOULCRUSHERS:
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
		case VENOMFALL_SPIRE:
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

static void take_tile(int r, int c) {
	if (game_board[c][r]->health <= game_board[col][row]->health) {	// attacking eliminates defending
		if ((game_board[col][row]->health -= game_board[c][r]->health) <= 0) { // attacking gets eliminated
			free(game_board[col][row]);
			game_board[col][row] = NULL;
		}
		free(game_board[c][r]);
		game_board[c][r] = NULL;
		if (game_board[col][row] != NULL) {	// attacking not eliminated, move to tile
			game_board[c][r] = game_board[col][row];
			game_board[col][row] = NULL;
			if (turn_state == CARD_MOVING) {
				row = r;
				col = c;
			}
		}
		tile_base_surfs[c][r].data = game_board[c][r]->img;
	} else {
		game_board[c][r]->health -= game_board[col][row]->health;
		free(game_board[col][row]);
		game_board[col][row] = NULL;
	}
}

bool attack_forward(void) {
	if ((col == 4 && player_state == P1) || (col == 0 && player_state == P2)) {
		base_health[player_state] = base_health[player_state] <= game_board[col][row]->health ?
			0 : base_health[player_state] - game_board[col][row]->health;
		free(game_board[col][row]);
		game_board[col][row] = NULL;
		return true;
	} else if (game_board[col + 1 - player_state * 2][row] != NULL
	 && game_board[col + 1 - player_state * 2][row]->player != player_state) {
		take_tile(col + 1 - player_state * 2, row);
		enable_intval_timer_interrupt();
		return true;
	}
	return false;
}

bool attack_sideways(void) {
	int r = row + (row < 2 ? 1 : -1);
	if (game_board[col][r] != NULL
	 && game_board[col][r]->player != player_state) {
		take_tile(col, r);
		enable_intval_timer_interrupt();
		return true;
	}
	r = row + (row < 2 ? -1 : 1);
	if (r >= 0 && r < ROW
	 && game_board[col][r] != NULL
	 && game_board[col][r]->player != player_state) {
		take_tile(col, r);
		enable_intval_timer_interrupt();
		return true;
	}
	return false;
}

void move_forward(void) {
	if (game_board[col + 1 - player_state * 2][row] != NULL) {
		game_board[col + 1 - player_state * 2][row] = game_board[col][row];
		game_board[col][row] = NULL;
		if (turn_state == CARD_MOVING) col += 1 - player_state * 2;
		enable_intval_timer_interrupt();
	}
}

void move_tode_the_elevated(void) {
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
		game_board[col][row]->health += TODE_THE_ELEVATED_HEAL;
		game_board[cur_cols[idx]][cur_rows[idx]] = game_board[col][row];
		game_board[col][row] = NULL;
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
			 && game_board[col][row]->poisoned);
			break;

		case MOMENTS_PEACE:
			return (game_board[col][row] != NULL
			 && game_board[col][row]->player == player_state);
			break;

		case SUMMON_MILITIA:
		case BLADE_STORM:
		case HEAD_START:
		case DARK_HARVEST:
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
				false,
				false,
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
				false,
				false,
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
				false,
				false,
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
				false,
				false,
				(player_state == P1) ? &dragon_p1 : &dragon_p2
			};
			break;

		case OPERATORS:
			*new_troop = (struct troop){
				card_id,
				UNIT,
				player_state,
				card_data[card_id].init_health,
				false,
				false,
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
				false,
				false,
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
				false,
				false,
				(player_state == P1) ? &toad_p1 : &toad_p2
			};
			break;

		case EMERALD_TOWERS:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				false,
				false,
				(player_state == P1) ? &emerald_p1 : &emerald_p2
			};
			break;

		case MOONLIT_AERIE:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				false,
				false,
				(player_state == P1) ? &moonlit_p1 : &moonlit_p2
			};
			break;

		case MECH_WORKSHOP:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				false,
				false,
				(player_state == P1) ? &workshop_p1 : &workshop_p2
			};
			break;

		case UPGRADE_POINT:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				false,
				false,
				(player_state == P1) ? &upgrade_p1 : &upgrade_p2
			};
			break;

		case VENOMFALL_SPIRE:
			*new_troop = (struct troop){
				card_id,
				BUILDING,
				player_state,
				card_data[card_id].init_health,
				false,
				false,
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
			place_new_tile_asset(row, col, new_troop);
			tile_overlay_surfs[col][row][++tile_overlay_surf_num[col][row]] = (struct surface){
				col2x(col),
				row2y(row),
				&on_attack
			};

			start_turn_action(row, col);
			break;

		case FELFLARES:
			place_new_tile_asset(row, col, new_troop);
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
					FELFLARES_DAMAGE,
					0
				};
			}
			break;

		case SUMMON_MILITIA:
			for (int i = 0; i <= (player_state == P1 ? front_columns[P1] : 5-front_columns[P2]); ++i) {
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
					SUMMON_MILITIA_SPAWN,
					KNIGHT
				};
			}
			break;

		case EXECUTION:
			health_change_list[++health_change_num] = (struct health_change){
				row,
				col,
				EXECUTION_DAMAGE,
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
							BLADESTORM_DAMAGE,
							0
						};
					}
				}
			}
			break;

		case DANGEROUS_SUITORS:
			place_new_tile_asset(row, col, new_troop);
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
					DANGEROUS_SUITORS_HEAL * heal,
					0
				};
			}
			break;

		case LUDIC_MATRIARCHS:
			place_new_tile_asset(row, col, new_troop);
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
						LUDIC_MATRIARCHEAD_START_SPAWN,
						DRAGON
					};
					cur_col = adj_col+bordering_col[i];
					cur_row = adj_row+bordering_row[i];
					if (game_board[cur_col][cur_row] == NULL) health_change_list[++health_change_num] = (struct health_change){
						cur_row,
						cur_col,
						LUDIC_MATRIARCHEAD_START_SPAWN,
						DRAGON
					};
				}
			}
			break;

		case DOPPELBOCKS:
			place_new_tile_asset(row, col, new_troop);
			int check_col = col+1-player_state*2;
			if (check_col >= 0 && check_col <= 4
			 && game_board[check_col][row] == NULL) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					check_col,
					DOPPELBOCKS_SPAWN,
					SATYR
				};
			}
			break;

		case HEAD_START:
			for (int i = 0; i < 4; ++i) {
				if (game_board[front_columns[player_state]][i] == NULL) {
					cur_rows[list_num++] = i;
				}
			}
			if (list_num > 0) {
				int idx = rand_num(0, list_num - 1);
				health_change_list[++health_change_num] = (struct health_change){
					cur_rows[idx],
					front_columns[player_state],
					HEAD_START_SPAWN,
					SATYR
				};
			}
			break;

		case DARK_HARVEST:
			for (int i = 0; i < COL; ++i) {
				for (int j = 0; j < ROW; ++j) {
					if (game_board[i][j] == NULL
					 || game_board[i][j]->player == player_state) continue;
					for (int k = i - 1; k <= i + 1; ++k) {
						int is_dmged = false;
						if (k < 0 || k > 4) continue;
						for (int l = j - 1; l <= j + 1; ++l) {
							if (l < 0 || l > 3 || (l == i && l == j)) continue;
							if (game_board[k][l] != NULL
							 && game_board[k][l]->player == player_state
							 && game_board[k][l]->type == UNIT) {
								health_change_list[++health_change_num] = (struct health_change){
									j,
									i,
									DARK_HARVEST_DAMAGE,
									0
								};
								is_dmged = true;
								break;
							}
						}
						if (is_dmged) break;
					}
				}
			}

			break;

		case FROSTHEXERS:
			place_new_tile_asset(row, col, new_troop);
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
			place_new_tile_asset(row, col, new_troop);
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
					FLESHMENDERS_HEAL,
					0
				};
			}
			break;

		case MOMENTS_PEACE:
			health_change_list[++health_change_num] = (struct health_change){
				row,
				col,
				MOMENTS_PEACE_HEAL,
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
			if (!game_board[col][row]->frozen) {
				status_change_list[++status_change_num] = (struct status_change){
					row,
					col,
					FROZEN
				};
			} else {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					col,
					ICICLE_BURST_DAMAGE,
					0
				};
			}
			break;

		case DR_MIA:
			place_new_tile_asset(row, col, new_troop);
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
			place_new_tile_asset(row, col, new_troop);
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
				MARKED_AS_PREY_DAMAGE,
				0
			};
			if (game_board[col][row]->health + MARKED_AS_PREY_DAMAGE <= 0) {
				health_change_list[++health_change_num] = (struct health_change){
					row,
					col,
					MARKED_AS_PREY_SPAWN,
					TOAD
				};
			}
			break;

		default:
			place_new_tile_asset(row, col, new_troop);
	}
}
