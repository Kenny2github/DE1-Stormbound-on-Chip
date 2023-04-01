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
	PRETURN,
	SELECT_CARD,
	PLACE_CARD,
	CONFIRM_CARD,
	CARD_MOVING
};

/**
 * @brief Enum of states of a card move
 */
enum move_state {
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

#endif
