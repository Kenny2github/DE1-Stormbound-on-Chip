/**
 * Data for instructions.txt
 */
#include "assets.h"

const char instructions_data[] =
	"                                  INSTRUCTIONS\n"
	"                     Before play, build a deck of 10 cards.\n"
	"    Player 1 starts with 3 mana and goes first; Player 2 starts with 4 mana.\n"
	" Initial turn mana increments each turn; Player 1 has 4 mana on their 2nd turn.\n"
	"    Each card costs a certain amount of mana to play. Unused mana is wasted.\n"
	"\n"
	"On turn start:                               | After turn start:\n"
	"1. Buildings' abilities take effect.         | 1. Play cards. Spells take effect\n"
	"2. Troops move forward one square, if        |    immediately, and \"on play\"\n"
	"   there is space for them to move into.     |    abilities are triggered.\n"
	"3. Troops that move into enemies attack them,| 2. Troops with nonzero movement\n"
	"   triggering any \"on attack\" abilities.     |    move. They will prefer to move\n"
	"4. The entity with greater strength survives |    into enemy troops and attack.\n"
	"   with its strength reduced by the other's, | 3. End turn.\n"
	"   triggering any \"on damage\" abilities.     +----------------------------------\n"
	"5. The entity with less strength dies, triggering any \"on death\" abilities.\n"
	"6. Surviving friendly troops trigger \"after attack\" abilities.\n"
	"\n"
	"Cards display up to 3 numbers: mana, strength, movement; for example, below are:\n"
	"\n"
	"          Felflares            |   Emerald Towers   |       Summon Militia\n"
	"3 mana, 2 strength, 0 movement | 4 mana, 4 strength |           1 mana"
;
