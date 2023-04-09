#ifndef GAME_H
#define GAME_H

#include "assets.h"
#include "image_data.h"

/**** Structs and Enums ****/

/**
 * @brief Info of card
 *
 * @param name String name of the card
 * @param faction Faction the card belongs to
 * @param img Image of the card
 * @param desc Description of the card
 * @param cost Mana cost of the card
 * @param init_health Initial health of the card
 * @param init_move Initial number of tiles the card moves
 */
struct card {
	char name[20];
	int faction;
	const struct image* img;
	const char* desc;
	int cost;
	int init_health;
	int init_move;
};

/**
 * @brief Info of troop
 *
 * @param card_id ID of the card, based on card_name
 * @param type Whether it's UNIT or BUILDING
 * @param player Owner of the troop
 * @param health Current health of the troop
 * @param status Current status effect of the troop
 * @param img Image of the troop on tile
 */
struct troop {
	int card_id;
	int type;
	int player;
	int health;
	int status;
	const struct image* img;
};

/**
 * @brief Enum of card IDs
 */
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
	MARKED_AS_PREY,
	KNIGHT,
	DRAGON,
	SATYR,
	CONSTRUCT,
	TOAD
};

/**
 * @brief Enum of card factions, for box display in deck building
 */
enum card_faction {
	NEUTRAL,
	SWARM,
	WINTER,
	IRONCLAD,
	SHADOWFEN
};

/**
 * @brief Enum of troop type, either UNIT or BUILDING
 */
enum troop_type {
	UNIT,
	BUILDING
};

/**
 * @brief Enum of status effects, either NO_STATUS, FROZEN, or POISONED
 */
enum status_type {
	NO_STATUS,
	FROZEN,
	POISONED
};

/**** Global Constants ****/

/**
 * @brief Const array of card items, ordered by card ID
 */
const struct card card_data[35] = {
	{"Lawless Herd", NEUTRAL, &lawless_herd, lawless_herd_desc_data, 2, 2, 0},
	{"Felflares", NEUTRAL, &felflares, felflares_desc_data, 3, 2, 0},
	{"Heroic Soldiers", NEUTRAL, &heroic_soldiers, heroic_soldiers_desc_data, 5, 6, 1},
	{"Victors of the Melee", NEUTRAL, &victors_of_the_melee, victors_of_the_melee_desc_data, 6, 4, 1},
	{"Emerald Towers", NEUTRAL, &emerald_towers, emerald_towers_desc_data, 4, 4, 0},
	{"Summon Militia", NEUTRAL, &summon_militia, summon_militia_desc_data, 1, 0, 0},
	{"Execution", NEUTRAL, &execution, execution_desc_data, 4, 0, 0},
	{"Blade Storm", NEUTRAL, &blade_storm, blade_storm_desc_data, 5, 0, 0},
	{"Dangeous Suitors", NEUTRAL, &dangerous_suitors, dangerous_suitors_desc_data, 6, 4, 1},
	{"Ludic Matriarchs", NEUTRAL, &ludic_matriarchs, ludic_matriarchs_desc_data, 6, 5, 0},
	{"Shady Ghoul", SWARM, &shady_ghoul, shady_ghoul_desc_data, 3, 1, 2},
	{"Doppelbocks", SWARM, &doppelbocks, doppelbocks_desc_data, 2, 1, 0},
	{"Moonlit Aerie", SWARM, &moonlit_aerie, moonlit_aerie_desc_data, 3, 3, 0},
	{"Head Start", SWARM, &head_start, head_start_desc_data, 2, 0, 0},
	{"Dark Harvest", SWARM, &dark_harvest, dark_harvest_desc_data, 5, 0, 0},
	{"Frosthexers", WINTER, &frosthexers, frosthexers_desc_data, 2, 1, 0},
	{"Wisp Cloud", WINTER, &wisp_cloud, wisp_cloud_desc_data, 3, 1, 1},
	{"Fleshmenders", WINTER, &fleshmenders, fleshmenders_desc_data, 7, 5, 2},
	{"Moment's Peace", WINTER, &moments_peace, moments_peace_desc_data, 5, 0, 0},
	{"Icicle Burst", WINTER, &icicle_burst, icicle_burst_desc_data, 1, 0, 0},
	{"Operators", IRONCLAD, &operators, operators_desc_data, 8, 12, 1},
	{"Dr. Mia", IRONCLAD, &dr_mia, dr_mia_desc_data, 2, 2, 0},
	{"Agents in Charge", IRONCLAD, &agents_in_charge, agents_in_charge_desc_data, 5, 3, 3},
	{"Mech Workshop", IRONCLAD, &mech_workshop, mech_workshop_desc_data, 4, 3, 0},
	{"Upgrade Point", IRONCLAD, &upgrade_point, upgrade_point_desc_data, 3, 4, 0},
	{"Copperskin Ranger", SHADOWFEN, &copperskin_ranger, copperskin_ranger_desc_data, 2, 1, 0},
	{"Soulcrushers", SHADOWFEN, &soulcrushers, soulcrushers_desc_data, 5, 5, 1},
	{"Tode the Elevated", SHADOWFEN, &tode_the_elevated, tode_the_elevated_desc_data, 4, 3, 1},
	{"Venomfall Spire", SHADOWFEN, &venomfall_spire, venomfall_spire_desc_data, 4, 4, 0},
	{"Marked as Prey", SHADOWFEN, &marked_as_prey, marked_as_prey_desc_data, 4, 0, 0},
	{"Knight", NEUTRAL, NULL, knight_desc_data, 0, 0, 0},
	{"Dragon", NEUTRAL, NULL, dragon_desc_data, 0, 0, 0},
	{"Satyr", SWARM, NULL, satyr_desc_data, 0, 0, 0},
	{"Construct", IRONCLAD, NULL, construct_desc_data, 0, 0, 0},
	{"Toad", SHADOWFEN, NULL, toad_desc_data, 0, 0, 0}
};

/**
 * @brief Const array of faction boxes for deck building
 */
const struct image* card_selection_box[5] = {
	&neutral_box,
	&swarm_box,
	&winter_box,
	&ironclad_box,
	&shadowfen_box
};

/**
 * @brief Array for iterating through bordering tiles
 */
const int bordering_row[4] = {0, -1, 1, 0};
/**
 * @brief Array for iterating through bordering tiles
 */
const int bordering_col[4] = {1, 0, 0, -1};


/**** Global Variables ****/

/**
 * @brief Current state of the board, filled with troops
 */
extern struct troop* game_board[5][4];
/**
 * @brief Currently selected row/col
 */
extern int row, col;
/**
 * @brief Current front of each player
 */
extern int front[2];

/**
 * @brief Current amount of mana left
 */
extern int mana;

/**
 * @brief Deck of both players, in order of appearance in hand
 */
extern int deck[2][10];
/**
 * @brief Currently selected card to be placed
 */
extern int cur_card_selected;


/**** Exported Functions ****/

/**
 * @brief Return random number between a and b (inclusive)
 */
int rand_num(int a, int b);

/**
 * @brief Swap two integers
 *
 * @param a Pointer to first int
 * @param b Pointer to second int
 */
void swap_int(int* a, int* b);

/**
 * @brief Initialize game states
 */
void init_game(void);

/**
 * @brief Game engine for handling logic
 */
void run_game(void);

#endif
