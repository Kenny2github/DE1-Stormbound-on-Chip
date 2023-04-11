#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "states.h"
#include "timer.h"
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

void display_base_health(void) {
	char base_health_text[3];
	base_health_text[0] = abs(base_health[P1]) / 10 + '0';
	base_health_text[1] = abs(base_health[P1]) % 10 + '0';
	base_health_text[2] = '\0';
	write_string(5, 17, base_health_text);
	base_health_text[0] = abs(base_health[P2]) / 10 + '0';
	base_health_text[1] = abs(base_health[P2]) % 10 + '0';
	write_string(73, 17, base_health_text);
}

void rerender_affected_tile() {
	if (!rerender_needed) return;

	r_stack_push(tile_base_surfs[affected_col][affected_row]);
	for (int i = 0; i < tile_overlay_surf_num[affected_col][affected_row]; ++i) {
		r_stack_push(tile_overlay_surfs[affected_col][affected_row][i]);
	}
	write_string((col2x(affected_col) + 20) / 4, (row2y(affected_row) + 13) / 4, "  \0");


	rerender_needed = false;
}

void push_health_change(int row, int col, int player, int change, enum card_name spawn_type) {
	health_change_list[health_change_num++] = (struct health_change) {
		row, col, player, change, spawn_type
	};
}

void push_status_change(int row, int col, enum status_type change) {
	status_change_list[status_change_num++] = (struct status_change) {
		row, col, change
	};
}

void change_healths() {
	if (health_change_idx == health_change_num) {
		health_change_num = 0;
		return;
	}
	char health_change_text[3];
	struct health_change cur_change = health_change_list[health_change_idx];
	if (cur_change.spawn_type >= KNIGHT) {
		struct troop* new_troop = (struct troop*) malloc(sizeof(struct troop));
		*new_troop = (struct troop){
			// spawn new unit of given type belonging to player
			cur_change.spawn_type, UNIT, cur_change.player,
			// give it health equal to the change in health
			cur_change.change,
			// it is not frozen or poisoned; get its image by type and player
			false, false, get_troop_img(cur_change.spawn_type, cur_change.player)
		};
		place_new_tile_asset(cur_change.row, cur_change.col, new_troop);
		rerender_needed = false;
	} else if (cur_change.change != 0) {
		if (cur_change.change < 0) {
			if (cur_change.change + game_board[cur_change.col][cur_change.row]->health < 0) {
				cur_change.change = -game_board[cur_change.col][cur_change.row]->health;
			}
			if (game_board[cur_change.col][cur_change.row]->health + cur_change.change == 0) {
				remove_tile_asset(cur_change.row, cur_change.col);
			} else {
				game_board[cur_change.col][cur_change.row]->health += cur_change.change;
				write_tile_health(cur_change.row, cur_change.col);
			}
			push_image(
				col2x(cur_change.col),
				row2y(cur_change.row),
				&damage
			);
		} else {
			if (cur_change.change + game_board[cur_change.col][cur_change.row]->health > 99) {
				cur_change.change = 99 - game_board[cur_change.col][cur_change.row]->health;
			}
			game_board[cur_change.col][cur_change.row]->health += cur_change.change;
			write_tile_health(cur_change.row, cur_change.col);
			push_image(
				col2x(cur_change.col),
				row2y(cur_change.row),
				&heal
			);
		}
		health_change_text[0] = abs(cur_change.change) / 10 + '0';
		health_change_text[1] = abs(cur_change.change) % 10 + '0';
		health_change_text[2] = '\0';
		write_string((col2x(cur_change.col) + 20) / 4, (row2y(cur_change.row) + 13) / 4, health_change_text);
		affected_row = cur_change.row;
		affected_col = cur_change.col;
		rerender_needed = true;
	}
	++health_change_idx;
	enable_intval_timer_interrupt();
}

void change_statuses() {
	if (status_change_idx == status_change_num) {
		status_change_num = 0;
		return;
	}
	struct status_change cur_change = status_change_list[status_change_idx];
	switch(cur_change.change) {
	case FREEZE:
		add_new_tile_overlay_asset(cur_change.row, cur_change.col, &frozen);
		game_board[cur_change.col][cur_change.row]->frozen = true;
		break;
	case CLEAR_FROZEN: ;	// assumes already frozen
		int i;
		for (i = 0; i < tile_overlay_surf_num[cur_change.col][cur_change.row]; ++i) {
			if (tile_overlay_surfs[cur_change.col][cur_change.row][i].data == &frozen) break;
		}
		for (; i < tile_overlay_surf_num[cur_change.col][cur_change.row] - 1; ++i) {
			tile_overlay_surfs[cur_change.col][cur_change.row][i].data = tile_overlay_surfs[cur_change.col][cur_change.row][i + 1].data;
		}
		--tile_overlay_surf_num[cur_change.col][cur_change.row];
		game_board[cur_change.col][cur_change.row]->frozen = false;
		affected_row = cur_change.row;
		affected_col = cur_change.col;
		rerender_needed = true;
	case POISON:
		add_new_tile_overlay_asset(cur_change.row, cur_change.col, &poisoned);
		game_board[cur_change.col][cur_change.row]->poisoned = true;
		break;
	}
	++status_change_idx;
	enable_intval_timer_interrupt();
}
