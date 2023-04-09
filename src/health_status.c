#include "game.h"
#include "health_status.h"

int base_health[2];

int health_change_num;
int health_change_idx;
struct health_change health_change_list[21];

int status_change_num;
int status_change_idx;
struct status_change status_change_list[20];

void change_healths() {
	if (health_change_num == 0) return;
}

void change_statuses() {
	return;
}
