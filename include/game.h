#ifndef GAME_H
#define GAME_H

#include "image_data.h"

struct card {
	char name[20];
	int type;
	struct image* img;
};

struct health_change {
	int row;
	int col;
	int change;
	int spawn_type;
};

struct status_change {
	int row;
	int col;
	int change;
};

enum card_name {
	LAWLESS_HERD,
	FELFLARES,
	HEROIC_SOLDIERS,
	VICTORS_OF_THE_MELEE,
	EMERALD_TOWERS,
	SUMMON_MILITIA,
	EXECUTION,
	BLADE_STORM,
	DANGEROUS_SUITORS,
	LUDIC_MATRIARCHS,
	SHADY_GHOUL,
	DOPPELBOCKS,
	MOONLIT_AERIE,
	HEAD_START,
	DARK_HARVEST,
	FROSTHEXERS,
	WISP_CLOUD,
	FLESHMENDERS,
	MOMENTS_PEACE,
	ICICLE_BURST,
	OPERATORS,
	DR_MIA,
	AGENTS_IN_CHARGE,
	MECH_WORKSHOP,
	UPGRADE_POINT,
	COPPERSKIN_RANGER,
	SOULCRUSHERS,
	TODE_THE_ELEVATED,
	VENOMFALL_SPIRE,
	MARKED_AS_PREY
};

enum card_type {
	NEUTRAL,
	SWARM,
	WINTER,
	IRONCLAD,
	SHADOWFEN
};


enum status_type {
	NO_STATUS,
	FROZEN,
	POISONED
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
