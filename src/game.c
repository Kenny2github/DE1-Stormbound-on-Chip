#include "assets.h"
#include "game.h"
#include "states.h"
#include "vga.h"

int game_state, turn_state, move_state, player_state;

void init_game() {
	game_state = TITLE;
	turn_state = PRETURN;
	move_state = CARD_EFFECT;
	player_state = P1;
}

void run_game() {
	if (game_state == TITLE) {
		fill_screen(BACKGROUND);
		draw_img_map(0, 100, stormbound);
		write_string(0, 0, "What the fuck did you just fucking say about me, you little bitch? I'll have\n"
						   "you know I graduated top of my class in the Navy Seals, and I've been involved\n"
						   "in numerous secret raids on Al-Quaeda, and I have over 300 confirmed kills. I\n"
						   "am trained in gorilla warfare and I'm the top sniper in the entire US armed\n"
						   "forces. You are nothing to me but just another target. I will wipe you the fuck\n"
						   "out with precision the likes of which has never been seen before on this Earth,\n"
						   "mark my fucking words. You think you can get away with saying that shit to me\n"
						   "over the Internet? Think again, fucker. As we speak I am contacting my secret\n"
						   "network of spies across the USA and your IP is being traced right now so you\n"
						   "better prepare for the storm, maggot. The storm that wipes out the pathetic\n"
						   "little thing you call your life. You're fucking dead, kid. I can be anywhere,\n"
						   "anytime, and I can kill you in over seven hundred ways, and that's just with my\n"
						   "bare hands. Not only am I extensively trained in unarmed combat, but I have\n"
						   "access to the entire arsenal of the United States Marine Corps and I will use it\n"
						   "to its full extent to wipe your miserable ass off the face of the continent, you\n"
						   "little shit. If only you could have known what unholy retribution your little\n"
						   "\"clever\" comment was about to bring down upon you, maybe you would have held\n"
						   "your fucking tongue. But you couldn't, you didn't, and now you're paying the\n"
						   "price, you goddamn idiot. I will shit fury all over you and you will drown in it.\n"
						   "You're fucking dead, kiddo.");
	}
}
