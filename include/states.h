#ifndef GAME_STATE_H
#define GAME_STATE_H

/**
 * @brief Enum of states of game
 */
enum game_state {
	TITLE,
	DECK,
	TURN
};

/**
 * @brief Enum of states of a player turn
 */
enum turn_state {
	PRETURN_BUILDING,
	PRETURN_UNIT,
	SELECT_CARD,
	PLACE_CARD,
	CARD_MOVING,
	TURN_END
};

/**
 * @brief Enum of states of a card move
 */
enum move_state {
	CARD_FIND_MOVE,
	CARD_EFFECT,
	CARD_MOVE
};

/**
 * @brief Enum of states of current player
 */
enum player_state {
	P1,
	P2
};

/**
 * @brief States of current game
 */
extern int game_state, turn_state, move_state, player_state;

#endif
