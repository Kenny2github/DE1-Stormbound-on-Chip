#ifndef TITLE_H
#define TITLE_H

#include "render.h"
#include "assets.h"

/**** Globals ****/

const struct surface intro_surfs[] = {
	{64, 5, &stormbound},
	{38, 120, &felflares},
	{148, 120, &emerald_towers},
	{248, 120, &summon_militia},
};
#define NUM_INTRO_SURFS (sizeof(intro_surfs) / sizeof(intro_surfs[0]))

/**** Exported Functions ****/

void draw_intro(void);

void run_title(void);

#endif
