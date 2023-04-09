#ifndef HEALTH_STATUS_H
#define HEALTH_STATUS_H

/**** Structs ****/

/**
 * @brief Info of health change
 *
 * @param row Row where health change occurs
 * @param col Column where health change occurs
 * @param change The amount of change to the tile
 * @param spawn_type If spawning a new unit, the type of unit
 */
struct health_change {
	int row;
	int col;
	int change;
	int spawn_type;
};

/**
 * @brief Info of status effect change
 *
 * @param row Row where status effect change occurs
 * @param col Column where status effect change occurs
 * @param change The status effect to set the unit at the tile to
 */
struct status_change {
	int row;
	int col;
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
 * @brief Goes through list of health changes and executes them
 */
void change_healths(void);

/**
 * @brief Goes through list of status effect changes and executes them
 */
void change_statuses(void);
#endif
