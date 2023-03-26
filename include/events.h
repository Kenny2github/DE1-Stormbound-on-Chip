/**
 * Event queue routines. These are agnostic to the event types,
 * which are defined in event_types.h.
 *
 * All interrupts should post an event to the event queue.
 * The event type msut be defined in event_types.h as documented there.
 */
#ifndef EVENTS_H
#define EVENTS_H

#include "event_types.h"

/**** Data structures ****/

/**
 * @brief Tag-value universal event data structure.
 */
struct event_t {
	// The tag for the type of event.
	enum event_type type;
	// The event data; of type described by the tag.
	union event_type_t data;
};

/**** Exported functions ****/

/**
 * @brief Is the event queue empty?
 *
 * @return 1 if empty, 0 if not
 */
int event_queue_empty(void);

/**
 * @brief Pop and return an event from the event queue.
 * This should only be called after testing !event_queue_empty().
 *
 * @return The event data.
 */
struct event_t event_queue_pop(void);

/**
 * @brief Push an event onto the event queue,
 * presumably from an interrupt handler.
 *
 * @param event The event data.
 * @param type_name Human-readable name of event type.
 */
void event_queue_push(struct event_t event, const char* type_name);

#endif
