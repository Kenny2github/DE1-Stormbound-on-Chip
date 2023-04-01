#include <stdio.h>
#include "address_map_arm.h"
#include "interrupts.h"
#include "mouse.h"
#include "timer.h"
#include "events.h"
#include "vga.h"
#include "game_state.h"

int seg7[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x063f};
int time_left;

static void config_KEYs(void) {
	volatile int* KEY_ptr = (int*)KEY_BASE;
	*(KEY_ptr + 2) = 0xF; // enable interrupts for the two KEYs
}

static void pushbutton_ISR(void) {
	volatile int* KEY_ptr = (int*)KEY_BASE;
	volatile int* HEX30_ptr = (int*)HEX3_HEX0_BASE;
	int press, HEX_bits;
	press = *(KEY_ptr + 3); // read the pushbutton interrupt register
	*(KEY_ptr + 3) = press; // Clear the interrupt

	if (press & 0x1) HEX_bits = 0b00111111; // KEY0
	else if (press & 0x2) HEX_bits = 0b00000110; // KEY1
	else if (press & 0x4) HEX_bits = 0b01011011; // KEY2
	else HEX_bits = 0b01001111; // KEY3

	*HEX30_ptr = HEX_bits;
}

static void handle_event(struct event_t event) {
	volatile int* LEDR_ptr = (int*) LEDR_BASE;
	switch (event.type) {
	case E_MOUSE_ENABLED:
		printf("Mouse plugged in\n");
		break;
	case E_MOUSE_BUTTON_DOWN:
		if (event.data.mouse_button_down.left) printf("Left button pressed\n");
		if (event.data.mouse_button_down.middle) printf("Middle button pressed\n");
		if (event.data.mouse_button_down.right) printf("Right button pressed\n");
		break;
	case E_MOUSE_BUTTON_UP:
		if (event.data.mouse_button_up.left) printf("Left button released\n");
		if (event.data.mouse_button_up.middle) printf("Middle button released\n");
		if (event.data.mouse_button_up.right) printf("Right button released\n");
		break;
	case E_MOUSE_MOVE:
		printf("Mouse moved to (%f, %f)\n", event.data.mouse_move.x, event.data.mouse_move.y);
		break;
	case E_TIMER_ENABLE:
		time_left = 10;
		printf("Timer enabled, time left = %d\n", time_left);
		*LEDR_ptr = (1 << time_left) - 1;
		break;
	case E_TIMER_RELOAD:
		if(!(--time_left)) {
			disable_timer();
			printf("timer disabled\n");			
			// put this somewhere else later, this is just for testing
			enable_timer_interrupt();
			
		} else {
			printf("timer counted down, time left = %d\n", time_left);
		}
		*LEDR_ptr = (1 << time_left) - 1;
	}
}

int main(void) {
	configure_vga();
	config_interrupt(IRQ_KEY, &config_KEYs, &pushbutton_ISR);
	enable_mouse();
	enable_timer();
	enable_timer_interrupt();
	config_interrupts();

	while (1) {
		while (!event_queue_empty()) {
			handle_event(event_queue_pop());
		}
		wait_for_vsync();
	}
}
