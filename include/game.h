#ifndef GAME_H
#define GAME_H

#include "assets.h"
#include "image_data.h"
#include "events.h"
#include "states.h"

#define NUM_CARDS 30
#define COL 5
#define ROW 4

/**** Structs and Enums ****/

/**
 * @brief Enum of card IDs
 */
enum card_name {
	LAWLESS_HERD = 0,
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

/**
 * @brief Card data for the deck or hand.
 */
struct card {
	// String name
	char name[20];
	// Faction the card belongs to
	enum card_faction faction;
	// Card image to render
	const struct image* img;
	// Card description to display
	const char* desc;
	// Mana cost
	int cost;
	// Health on play
	int init_health;
	// Number of tiles to move on play
	int init_move;
};

/**
 * @brief Troop data for the board.
 */
struct troop {
	// Which card does this troop relate to?
	enum card_name card_id;
	// Is this troop a unit or a building?
	enum troop_type type;
	// Which player does this troop belong to?
	enum player_state player;
	// Current health
	int health;
	// Is this troop frozen?
	bool frozen;
	// Is this troop poisoned?
	bool poisoned;
	// Tile image to render
	const struct image* img;
};

/**** Global Constants ****/

extern const char* empty_desc_data;

/**
 * @brief Const array of card items, ordered by card ID
 */
extern const struct card card_data[35];

/**
 * @brief Const array of faction boxes for deck building
 */
extern const struct image* card_selection_box[5];

/**
 * @brief Array for iterating through bordering tiles
 */
extern const int bordering_row[4];
/**
 * @brief Array for iterating through bordering tiles
 */
extern const int bordering_col[4];


/**** Global Variables ****/

/**
 * @brief Current state of the board, filled with troops
 */
extern struct troop* game_board[COL][ROW];

/**
 * @brief Currently selected row/col
 */
extern int row, col;

/**
 * @brief Current frontmost column each player has units/buildings on.
 */
extern int front_columns[2];

/**
 * @brief Current round number. Determines starting mana.
 * Counts up from 0 after the ending of each P1 turn.
 */
extern int cur_round;

/**
 * @brief Mana remaining for current player.
 * Reset to cur_round + 3 at beginning of each turn.
 */
extern int mana;

/**
 * @brief Deck of both players, in order of appearance in hand.
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
 * @brief Convert a tile column to a pixel x-coordinate.
 * The pixel coordinate corresponds to the top-left of the tile.
 *
 * @param c The tile column, 0-4 left to right.
 * @return Pixel x-coordinate.
 */
int col2x(int c);

/**
 * @brief Convert a tile row to a pixel y-coordinate.
 * The pixel coordinate corresponds to the top-left of the tile.
 *
 * @param r The tile row, 0-3 top to bottom.
 * @return Pixel y-coordinate.
 */
int row2y(int r);

/**
 * @brief Convert a pixel x-coordinate to a tile column.
 * Any pixel coordinate inside the tile will return the same result.
 *
 * @param x Pixel x-coordinate.
 * @return The tile column, 0-4 left to right.
 */
int x2col(int x);

/**
 * @brief Convert a pixel y-coordinate to a tile row.
 * Any pixel coordinate inside the tile will return the same result.
 *
 * @param y Pixel y-coordinate.
 * @return The tile row, 0-3 top to bottom.
 */
int y2row(int y);

/**
 * @brief Common event handlers unspecific to any context.
 * This should be called in the event loop in addition to other event handling.
 *
 * @param event The event to handle.
 */
void default_event_handlers(struct event_t event);

/**
 * @brief Initialize game states.
 */
void init_game(void);

/**
 * @brief Run one frame of the game.
 */
void run_game(void);

#endif
