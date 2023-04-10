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

int affected_row, affected_col;
bool rerender_needed;

void reset_health_status_changes(void) {
	health_change_idx = 0;
	health_change_num = 0;
	status_change_idx = 0;
	status_change_num = 0;
	rerender_needed = false;
}

void rerender_affected_tile() {
	if (!rerender_needed) return;

	r_stack_push(tile_base_surfs[affected_col][affected_row]);
	for (int i = 0; i < tile_overlay_surf_num[affected_col][affected_row]; ++i) {
		r_stack_push(tile_overlay_surfs[affected_col][affected_row][i]);
	}

	rerender_needed = false;
}

void change_healths() {
	if (health_change_idx == health_change_num) {
		health_change_num = 0;
		return;
	}
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
		rerender_needed = false;
	} else {
		char health_change_text[1];
		if (cur_change.change < 0) {
			push_image(
				col2x(cur_change.col),
				row2y(cur_change.row),
				&damage
			);
			if (cur_change.change + game_board[cur_change.col][cur_change.row]->health < 0) {
				cur_change.change = -game_board[col][cur_change.row]->health;
			}
		} else {
			push_image(
				col2x(cur_change.col),
				row2y(cur_change.row),
				&heal
			);
			if (cur_change.change + game_board[cur_change.col][cur_change.row]->health > 99) {
				cur_change.change = 99 - game_board[col][cur_change.row]->health;
			}
		}
		health_change_text[0] = abs(cur_change.change) + '0';
		write_string((col2x(cur_change.col) + 40) / 4, (row2y(cur_change.row) + 26) / 4, health_change_text);
		affected_row = cur_change.row;
		affected_col = cur_change.col;
		rerender_needed = true;
	}
	++health_change_idx;
}

void change_statuses() {
	return;
}
