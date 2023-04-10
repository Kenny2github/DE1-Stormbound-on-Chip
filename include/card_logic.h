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
 * @brief Attacks the troop directly in front
 *
 * @return Whether an attack was performed
 */
bool attack_forward(void);

/**
 * @brief Attacks the troop on the side
 *
 * @return Whether an attack was performed
 */
bool attack_sideways(void);

/**
 * @brief Move troop forward
 */
void move_forward(void);

/**
 * @brief Additional movement logic just for Tode the Elevated.
 */
void move_tode_the_elevated(void);

/**
 * @brief Return whether a card can be placed there
 */
bool valid_play_card(void);

/**
 * @brief Perform on-play action for troop
 */
void play_card(void);

#endif
