#ifndef DECKBUILDING_H
#define DECKBUILDING_H

#include "game.h"
#include "render.h"

/**** Constatns ****/

extern const char* empty_card_name_data;

/**** Globals ****/

extern int card_num;
extern bool in_deck[30];

struct surface deckbuilding_card_surfs[NUM_CARDS];
struct surface deckbuilding_deck_surfs[10];
bool deckbuilding_displaying;
int deckbuilding_cur_display;
bool deckbuilding_done_displaying;

/**** Exported Functions ****/

void init_deckbuilding(void);

void run_deckbuilding(void);

#endif
