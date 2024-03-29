#ifndef TURN_H
#define TURN_H

#include <stdbool.h>
#include "game.h"

enum board_sprites {
	BASE_P1,
	BASE_P2,
	TILES,
	FRONT_P1,
	FRONT_P2
};

/**** Globals ****/

extern bool cur_cards_played[4];
extern int cur_card_selected;

/**
 * @brief Determines whether an intial move is waived or not
 * Needed for TTE / SG logic
 */
extern bool move_waived[COL][ROW];

extern struct surface tile_base_surfs[COL][ROW];
extern struct surface tile_overlay_surfs[COL][ROW][4];
extern int tile_overlay_surf_num[COL][ROW];

/**
 * @brief Surface objects of the board.
 * In order: P1 base, P2 base, tiles, P1 front, P2 front
 */
extern struct surface board_base_surfs[5];

extern struct surface cur_card_deck_surfs[4];

extern struct surface new_turn_surf;

/**** Exported Functions ****/

void update_mana(int new_mana);
void update_front(int player);
void init_turn(void);
void run_turn(void);

#endif
