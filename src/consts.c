#include <stdlib.h>
#include "assets.h"
#include "game.h"
#include "deckbuilding.h"

const char* empty_desc_data =
	"                        \n"
	"                        \n"
	"                        \n"
	"                        \n"
	"                        \n"
	"                        \n"
	"                        \n"
;

const char* empty_card_name_data =
	"                    "
;

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
	{"Token Knight", NEUTRAL, NULL, knight_desc_data, 0, 0, 0},
	{"Token Dragon", NEUTRAL, NULL, dragon_desc_data, 0, 0, 0},
	{"Token Satyr", SWARM, NULL, satyr_desc_data, 0, 0, 0},
	{"Token Construct", IRONCLAD, NULL, construct_desc_data, 0, 0, 0},
	{"Token Toad", SHADOWFEN, NULL, toad_desc_data, 0, 0, 0}
};

const struct image* card_selection_box[5] = {
	&neutral_box,
	&swarm_box,
	&winter_box,
	&ironclad_box,
	&shadowfen_box
};

const int bordering_row[4] = {0, -1, 1, 0};
const int bordering_col[4] = {1, 0, 0, -1};
