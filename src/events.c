/**
 * Event queue routines.
 *
 * We implement the event queue as a ring-buffer-style FIFO queue.
 * By using native word size, operations should be interrupt-safe.
 */
#include <stdio.h>
#include <stdlib.h>
#include "events.h"

#define EVENT_QUEUE_LEN 128

/**** Global variables ****/

static volatile size_t q_head;
static volatile size_t q_tail;
static volatile struct event_t q_data[EVENT_QUEUE_LEN];

/**** Exported functions ****/

int event_queue_empty(void) {
	return q_head == q_tail;
}

struct event_t event_queue_pop(void) {
	// N.B. done under the assumption that event_queue_empty().
	struct event_t event = q_data[q_tail];
	q_tail = (q_tail + 1) % EVENT_QUEUE_LEN;
	return event;
}

void event_queue_push(struct event_t event, const char* type_name) {
	size_t next_head = (q_head + 1) % EVENT_QUEUE_LEN;
	if (next_head == q_tail) {
		printf("Warning: %s event dropped due to full queue\n", type_name);
		return;
	}
	q_data[q_head] = event;
	q_head = next_head;
}
