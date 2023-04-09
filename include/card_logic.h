#ifndef CARD_LOGIC_H
#define CARD_LOGIC_H

/**** Exported Functions ****/


void place_new_tile_asset(int r, int c, struct troop* new_troop);

void remove_tile_asset(int r, int c);

/**
 * @brief Perform start-of-turn action for troop located at coordinates
 *
 * @param act_row Row where troop is located
 * @param act_col Column where troop is located
 */
void start_turn_action(int act_row, int act_col);

/**
 * @brief Move troop forward, dealing damage if necessary
 */
void move_forward(void);

/**
 * @brief Return whether a card can be placed there
 */
bool valid_play_card(void);

/**
 * @brief Perform on-play action for troop
 */
void play_card(void);

#endif
