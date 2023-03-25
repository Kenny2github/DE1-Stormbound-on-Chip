#include "address_map_arm.h"
#include "interrupts.h"
#include "mouse.h"

int seg7[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x063f};

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

int main(void) {
	config_interrupt(IRQ_KEY, &config_KEYs, &pushbutton_ISR);
	enable_mouse();
	config_interrupts();

	while (1); // wait for interrupts
}
