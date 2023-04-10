#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "states.h"
#include "vga.h"
#include "render.h"
#include "turn.h"
#include "card_logic.h"
#include "health_status.h"

int base_health[2];

int health_change_num;
int health_change_idx;
struct health_change health_change_list[21];

int status_change_num;
int status_change_idx;
struct status_change status_change_list[20];

void change_healths() {
	if (health_change_num == 0) return; // nothing to do

	struct health_change cur_change = health_change_list[health_change_idx];
	if (cur_change.spawn_type >= KNIGHT) {
		struct troop* new_troop = (struct troop*) malloc(sizeof(struct troop));
		*new_troop = (struct troop){
			// spawn new unit of given type belonging to current player
			cur_change.spawn_type, UNIT, player_state,
			// give it health equal to the change in health
			cur_change.change,
			// it is not frozen or poisoned; get its image by type and player
			false, false, get_troop_img(cur_change.spawn_type, player_state)
		};
		place_new_tile_asset(cur_change.row, cur_change.col, new_troop);
	} else {
		char health_change_text[1];
		if (cur_change.change < 0) {
			tile_overlay_surfs[cur_change.col]
							   [cur_change.row]
							   [++tile_overlay_surf_num[cur_change.col][cur_change.row]]
			= (struct surface){
				col2x(cur_change.col),
				row2y(cur_change.row),
				&damage
			};
			if (cur_change.change + game_board[cur_change.col][cur_change.row]->health < 0) {
				cur_change.change = -game_board[col][cur_change.row]->health;
			}
		} else {
			tile_overlay_surfs[cur_change.col]
							   [cur_change.row]
							   [++tile_overlay_surf_num[cur_change.col][cur_change.row]]
			= (struct surface){
				col2x(cur_change.col),
				row2y(cur_change.row),
				&heal
			};
			if (cur_change.change + game_board[cur_change.col][cur_change.row]->health > 99) {
				cur_change.change = 99 - game_board[col][cur_change.row]->health;
			}
		}
		health_change_text[0] = abs(cur_change.change) + '0';
		write_string((col2x(cur_change.col) + 40) / 4, (row2y(cur_change.row) + 26) / 4, health_change_text);
	}
}

void change_statuses() {
	return;
}
