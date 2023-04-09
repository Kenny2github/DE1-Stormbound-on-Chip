#ifndef TURN_H
#define TURN_H

#include "game.h"

/**** Globals ****/

extern bool cur_cards_played[4];
extern int cur_card_selected;

struct surface board_base_surfs[COL+3][ROW];	// extra 3 is for board from left to right
struct surface board_overlay_surfs[COL][ROW][4];	// extra 2 is for the bases
int board_overlay_surf_num[COL][ROW];

struct surface cur_card_deck_surfs[4];

/**** Exported Functions ****/

void update_mana(int new_mana);
void update_front(void);
void init_turn(void);
void run_turn(void);

#endif
