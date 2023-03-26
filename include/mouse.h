/**
 * Routines for mouse events and tracking.
 */
#ifndef MOUSE_H
#define MOUSE_H

/**** Data structures ****/

/**
 * @brief Mouse state data.
 */
struct mouse_state_t {
	// The current mouse x position (right is positive)
	float x;
	// The current mouse y position (down is positive)
	float y;
	// Whether the left mouse button is currently clicked
	unsigned int left_clicked : 1;
	// Whether the middle mouse button is currently clicked
	unsigned int middle_clicked : 1;
	// Whether the right mouse button is currently clicked
	unsigned int right_clicked : 1;
};

/**** Global variables ****/

/**
 * @brief The current state of the mouse.
 * Updated by interrupts, beware.
 */
extern struct mouse_state_t mouse_state;

/**** Exported functions ****/

/**
 * @brief Enable mouse functionality.
 *
 * This sets up interrupts and such to track the mouse state.
 * Must be called before accessing mouse_state.
 */
void enable_mouse(void);

#endif
