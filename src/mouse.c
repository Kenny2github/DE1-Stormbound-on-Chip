/**
 * Routines for mouse events and tracking.
 *
 * We listen to PS/2 interrupts and use them to track the current:
 * - position, and
 * - button state
 * of the mouse.
 *
 * Many internal helpers are static and should not be exported.
 */
#include <stdio.h>
#include "address_map_arm.h"
#include "interrupts.h"
#include "mouse.h"
#include "vga.h"

#define DEBUG_PRINT

// Mouse sensitivity constant. Used to slow down mouse movement if needed.
#define MOUSE_SENSITIVITY 0.125

struct mouse_state_t mouse_state;

/**
 * @brief Mouse-to-host commands.
 */
enum PS2_recv {
	ACKNOWLEDGE = 0xFA,
	SELF_TEST_PASSED = 0xAA,
	SELF_TEST_FAILED = 0xFC,
};
/**
 * @brief Host-to-mouse commands.
 */
enum PS2_send {
	RESET = 0xFF,
	RESET_WITHOUT_SELF_TEST = 0xF6,
	RESEND_LAST_BYTE = 0xFE,
	ENABLE_DATA_REPORTING = 0xF4,
	DISABLE_DATA_REPORTING = 0xF5,
};
/**
 * @brief States for the finite state machine that initializes the mouse.
 */
enum mouse_reporting_status {
	// Mouse has not yet initialized
	NOT_REPORTING = 0,
	// Mouse has been requested to begin data reporting;
	// now waiting for it to acknowledge the request
	AWAITING_ACK,
	// Mouse is now reporting data
	REPORTING
};

/**
 * @brief Enable interrupts on the PS/2 port.
 */
static void enable_mouse_interrupts(void) {
	volatile int* PS2_ptr = (int*)PS2_BASE;
	// Write 1 to RE bit of control register
	*(PS2_ptr + 1) = 1;
}

/**
 * @brief Update mouse state from a PS/2 mouse state report packet.
 *
 * @param packet PS/2 mouse state report packet
 */
static void update_mouse_state(char packet[3]) {
	// Update mouse buttons
	mouse_state.left_clicked = packet[0] & 0b001;
	mouse_state.right_clicked = (packet[0] & 0b010) >> 1;
	mouse_state.middle_clicked = (packet[0] & 0b100) >> 2;
	// construct 9-bit signed numbers
	struct {
		signed short x : 9;
		signed short y : 9;
	} dpos;
	dpos.x = (((signed short)packet[0] & 0x10) << 4) | (signed short)packet[1];
	dpos.y = (((signed short)packet[0] & 0x20) << 3) | (signed short)packet[2];
	// sign-extend 9 bits to 16
	signed short dx = dpos.x;
	signed short dy = dpos.y;
	// update mouse x position
	mouse_state.x += dx * MOUSE_SENSITIVITY;
	mouse_state.y += dy * MOUSE_SENSITIVITY;
	if (mouse_state.x < 0) mouse_state.x = 0;
	else if (mouse_state.x >= SCREEN_W) mouse_state.x = SCREEN_W - 1;
	if (mouse_state.y < 0) mouse_state.y = 0;
	else if (mouse_state.y >= SCREEN_H) mouse_state.y = SCREEN_H - 1;

#ifdef DEBUG_PRINT
	printf(
		"Mouse x: %f, y: %f, dx: %hd, dy: %hd, L: %u, M: %u, R: %u\n",
		mouse_state.x, mouse_state.y, dx, dy, mouse_state.left_clicked,
		mouse_state.middle_clicked, mouse_state.right_clicked
	);
#endif
}

/**
 * @brief Handle an interrupt from the PS/2 port.
 *
 * This operates on a state machine with transition diagram as follows:
 * Init
 *   |
 *   V
 * NOT_REPORTING : ignore all packets except --
 *   | if a packet is SELF_TEST_PASSED,
 *   V request to enable data reporting.
 * AWAITING_ACK : ignore all packets except --
 *   | if a packet is ACKNOWLEDGE.
 *   V
 * REPORTING : process reported packets
 */
static void handle_mouse_interrupt(void) {
	// 3-byte mouse packet
	static char packet[3];
	// Counts 0, 1, 2, packet is complete on 0
	static char packet_completion = 0;
	// Current data reporting status of the mouse
	static enum mouse_reporting_status status = NOT_REPORTING;

	volatile int* PS2_ptr = (int*)PS2_BASE;
	// Whether the PS/2 FIFO data is valid
	unsigned int fifo_valid;

	while (1) {
		// Read from FIFO. Doing this enough times clears the interrupt.
		unsigned int PS2_data = *PS2_ptr;
		// Check RVALID bit
		fifo_valid = PS2_data & 0x00008000;
		if (!fifo_valid) break;

		// Shift packet left
		packet[0] = packet[1];
		packet[1] = packet[2];
		packet[2] = PS2_data & 0x000000FF;
#ifdef DEBUG_PRINT
		printf("%02x %02x %02x\n", packet[0], packet[1], packet[2]);
#endif

		// Not yet in 3-byte-packet streaming-data-reporting mode
		if (status != REPORTING) {
			if (packet[1] == (char)SELF_TEST_PASSED && packet[2] == (char)0x00) {
				// Request to enter data reporting mode
				*PS2_ptr = ENABLE_DATA_REPORTING;
				status = AWAITING_ACK;
			} else if (status == AWAITING_ACK && packet[2] == (char)ACKNOWLEDGE) {
				// Above request was acknowledged
				status = REPORTING;
			}
			continue;
		}
		// Packet is a mouse state packet

		packet_completion = (packet_completion + 1) % 3;
		// Don't process incomplete packet
		if (packet_completion != 0) continue;

		update_mouse_state(packet);
	}
}

void enable_mouse(void) {
	config_interrupt(IRQ_PS2, &enable_mouse_interrupts, &handle_mouse_interrupt);
}
