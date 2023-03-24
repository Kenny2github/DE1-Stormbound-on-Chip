#include "address_map_arm.h"

int seg7[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x063f};

int main(void) {
	volatile int* SW_ptr = SW_BASE;
	volatile int* HEX30_ptr = HEX3_HEX0_BASE;
	int value, count;

	while (1) {
		value = *SW_ptr;
		count = 0;
		while (value != 0) {
			if (value & 1) ++count;
			value = value >> 1;
		}
		*HEX30_ptr = seg7[count];
	}
}
