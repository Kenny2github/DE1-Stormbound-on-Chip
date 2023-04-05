#ifndef GAME_H
#define GAME_H

#include "image_data.h"

struct card {
	char name[20];
	int type;
	struct image* img;
};

enum card_type {
	NEUTRAL,
	SWARM,
	WINTER,
	IRONCLAD,
	SHADOWFEN
};

/**
 * @brief Initialize game states
 */
void init_game(void);

/**
 * @brief Game engine for handling logic
 */
void run_game(void);

#endif
