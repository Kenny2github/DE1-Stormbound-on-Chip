/**
 * List of event types. All events on the event queue
 * must have their type defined here.
 *
 * Defining a new event type involves adding the following:
 * - The event-specific data structure.
 * - An entry to the event_type enum uniquely identifying the event.
 * - An entry to the event_type_t union for the event-specific data structure.
 */
#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

/**** Event type data structures ****/

/**
 * @brief Empty Data structure.
 * This is empty - all that's needed is the event type.
 */
struct e_empty {};

/**
 * @brief Data structure for a mouse movement event.
 */
struct e_mouse_move {
	// The old x-coordinate of the mouse.
	float old_x;
	// The old y-coordinate of the mouse.
	float old_y;
	// The new x-coordinate of the mouse.
	float x;
	// The new y-coordinate of the mouse.
	float y;
};

/**
 * @brief Data structure for a mouse button event.
 * This structure is used for both mouse button down and up events.
 */
struct e_mouse_button {
	// If set, the left mouse button was clicked/released.
	unsigned int left : 1;
	// If set, the middle mouse button was clicked/released.
	unsigned int middle : 1;
	// If set, the right mouse button was clicked/released.
	unsigned int right : 1;
};

/**** Event type enum and union ****/

/**
 * @brief Enum of possible event types. This is used
 * to check what type of event the event is.
 */
enum event_type {
	E_MOUSE_ENABLED,
	E_MOUSE_MOVE,
	E_MOUSE_BUTTON_DOWN,
	E_MOUSE_BUTTON_UP,
	E_TIMER_ENABLE,
	E_TIMER_RELOAD
};

/**
 * @brief Union of possible event data structures.
 * This stores the actual data of the event.
 */
union event_type_t {
	struct e_empty mouse_enabled;
	struct e_mouse_move mouse_move;
	struct e_mouse_button mouse_button_down;
	struct e_mouse_button mouse_button_up;
	struct e_empty timer_enable;
	struct e_empty timer_reload;
};

#endif
