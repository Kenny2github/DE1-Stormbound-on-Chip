#ifndef HEALTH_STATUS_H
#define HEALTH_STATUS_H

#include "game.h"

/**** Structs ****/

/**
 * @brief A queued up health change for a unit.
 */
struct health_change {
	// Position where health change occurs.
	int row, col;
	// Amount by which to change health.
	int change;
	// If there is no troop at that location,
	// the health change will spawn one.
	// This determines the type of troop to spawn.
	enum card_name spawn_type;
};

/**
 * @brief A queued up status effect change for a unit.
 */
struct status_change {
	// Position where status effect change occurs.
	int row, col;
	// Change to make (to be defined)
	int change;
};

/**** Global Variables ****/

/**
 * @brief Health of each player's base
 */
int base_health[2];

/**
 * @brief Total number of pending health changes
 */
extern int health_change_num;
/**
 * @brief Current index of health change
 */
extern int health_change_idx;
/**
 * @brief List of health changes to be made
 */
extern struct health_change health_change_list[21];

/**
 * @brief Total number of pending status effect changes
 */
extern int status_change_num;
/**
 * @brief Current index of status effect change
 */
extern int status_change_idx;
/**
 * @brief List of status effect changes to be made
 */
extern struct status_change status_change_list[20];


/**** Exported Functions ****/

/**
 * @brief Process pending health changes.
 */
void change_healths(void);

/**
 * @brief Process pending status effect changes.
 */
void change_statuses(void);

#endif
