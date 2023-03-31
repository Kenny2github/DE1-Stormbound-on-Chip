#ifndef GAME_STATE_H
#define GAME_STATE_H

/**
 * @brief Enum of states of game. This is used
 * to determine how to handle input and output.
 */
enum game_state {
	TITLE,
	P1_DECK,
	P2_DECK,
	P1_PRETURN,
	P1_TURN,
	P2_PRETURN,
	P2_TURN
};

#endif
