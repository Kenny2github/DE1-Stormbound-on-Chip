#ifndef CARD_LOGIC_H
#define CARD_LOGIC_H

/**** Exported Functions ****/

/**
 * @brief Get tile image for a troop based on its type.
 *
 * @param card The card to get the image for.
 * @param player The player who owns the card.
 * @return The image to render on the board.
 */
struct image* get_troop_img(enum card_name card, enum player_state player);

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
 * @brief Determine next tile troop moves to based on FIO
 *
 * @param r Row troop is currently on
 * @param c Column troop is currently on
 * @param next_r Pointer to row troop will move to
 * @param next_c Pointer to column troop will move to
 */
void find_next_move(int r, int c, int* next_r, int* next_c);

/**
 * @brief Move troop to new tile, dealing damage if necessary
 *
 * @return Whether an attack was performed
 * @param r Pointer to row troop is currently on
 * @param c Pointer to column troop is currently on
 * @param new_r Row troop will move to
 * @param new_c Column troop will move to
 */
bool move_to_tile(int* r, int* c, int new_r, int new_c);

/**
 * @brief Additional movement logic just for Tode the Elevated.
 */
void find_tode_the_elevated_jump(int r, int c, int* next_r, int* next_c);

/**
 * @brief Return whether a card can be placed there
 */
bool valid_play_card(void);

/**
 * @brief Perform on-play action for troop
 */
void play_card(void);

#endif
