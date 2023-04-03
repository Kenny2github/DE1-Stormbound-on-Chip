/**
 * Routines for configuring interrupts.
 * Many internal helpers are static and should not be exported.
 *
 * Adapted from "Using the ARM Generic Interrupt Controller",
 * copyright Intel 2018.
 */
#include <stdlib.h>
#include "interrupts.h"
#include "address_map_arm.h"

// The maximum number of interrupts allowed.
// Some buffer space is left beyond the number of IRQs listed on CPUlator.
#define MAX_INTERRUPTS 20

/**** Data structures ****/

/**
 * @brief A single interrupt handler.
 */
struct interrupt_handler {
	// The interrupt ID to handle.
	int interrupt_id;
	// I/O device interrupt enabler function pointer.
	void (*enable)(void);
	// Interrupt handler function pointer.
	void (*handler)(void);
};

/**** Global variables ****/

/**
 * @brief The number of currently registered interrupt handlers.
 * Must be at most MAX_INTERRUPTS.
 */
static int num_interrupts = 0;
/**
 * @brief Array of interrupt handlers. Most will be unused.
 */
static struct interrupt_handler interrupt_handlers[MAX_INTERRUPTS];

/**** Static functions ****/

/**
 * @brief Turn off interrupts in the ARM processor.
 */
static void disable_A9_interrupts(void) {
	int status = 0b11010011;
	asm("MSR CPSR, %[ps]" : : [ps] "r"(status));
}

/**
 * @brief Initialize the banked stack pointer register for IRQ mode.
 */
static void set_A9_IRQ_stack(void) {
	// The top of A9 onchip memory, aligned to 8 bytes.
	int stack = 0xFFFFFFFF - 7;
	int mode = 0b11010010; // Processor mode
	// Change processor to IRQ mode with interrupts disabled
	asm("MSR CPSR, %[ps]" : : [ps] "r"(mode));
	// Set banked stack pointer
	asm("MOV SP, %[ps]" : : [ps] "r"(stack));
	// Go back to SVC mode before executing subroutine returnn
	mode = 0b11010011;
	asm("MSR CPSR, %[ps]" : : [ps] "r"(mode));
}

/**
 * @brief Turn on interrupts in the ARM processor.
 */
static void enable_A9_interrupts(void) {
	int status = 0b01010011;
	asm("MSR CPSR, %[ps]" : : [ps] "r"(status));
}

/**
 * @brief Configure Set Enable Registers (ICDISERn) and Interrupt Processor
 * Target Registers (ICDIPTRn). The default (reset) values are used for other
 * registers in the GIC.
 *
 * @param N The interrupt ID to configure.
 * @param CPU_target The CPU ID to target.
 */
static void enable_interrupt(int N, int CPU_target) {
	int reg_offset, index, value, address;

	/* Configure the Interrupt Set-Enable Registers (ICDISERn).
	 * reg_offset = integer_div(N / 32) * 4
	 * value = 1 << (N mod 32)
	 */
	reg_offset = (N >> 3) & 0xFFFFFFFC;
	index = N & 0x1F;
	value = 0x1 << index;
	address = MPCORE_GIC_DIST + ICDISER + reg_offset;
	// Now that we know the register address and value, set the appropriate bit.
	*(int*)address |= value;

	/* Configure the Interrupt Processor Targets Register (ICDIPTRn).
	 * reg_offset = integer_div(N / 4) * 4
	 * index = N mod 4
	 */
	reg_offset = N & 0xFFFFFFFC;
	index = N & 0x3;
	address = MPCORE_GIC_DIST + ICDIPTR + reg_offset + index;
	// Now that we know the register address and value,
	// write to (only) the appropriate byte.
	*(char*)address = (char)CPU_target;
}

/**
 * @brief Configure the Generic Interrupt Controller
 */
static void config_GIC(void) {
	for (int i = 0; i < num_interrupts; ++i) {
		enable_interrupt(interrupt_handlers[i].interrupt_id, 1);
	}

	// Set Interrupt Priority Mask Register (ICCPMR).
	// Enable interrupts of all priorities
	*((int*)(MPCORE_GIC_CPUIF + ICCPMR)) = 0xFFFF;

	// Set CPU Interface Control Register (ICCICR).
	// Enable signaling of interrupts
	*((int*)(MPCORE_GIC_CPUIF + ICCICR)) = 1;

	// Configure the Distributor Control Register (ICDDCR)
	// to send pending interrupts to CPUs
	*((int*)(MPCORE_GIC_DIST + ICDDCR)) = 1;
}

/**** Exception handlers ****/

// IRQ exception handler
void __attribute__((interrupt)) __cs3_isr_irq(void) {
	// Read the ICCIAR from the CPU Interface in the GIC
	int interrupt_id = *((int*)(MPCORE_GIC_CPUIF + ICCIAR));
	int i;

	// Loop through registered interrupt handlers
	for (i = 0; i < num_interrupts; ++i) {
		if (interrupt_id == interrupt_handlers[i].interrupt_id) {
			// This interrupt ID matches; call its handler
			(*interrupt_handlers[i].handler)();
			break;
		}
	}

	// No handler matched.
	if (i >= num_interrupts) while (1);

	// Write to the End of Interrupt Register (ICCEOIR)
	*((int*)(MPCORE_GIC_CPUIF + ICCEOIR)) = interrupt_id;
}

// Other exception handlers

void __attribute__((interrupt)) __cs3_reset(void) {
	while(1);
}

void __attribute__((interrupt)) __cs3_isr_undef(void) {
	while(1);
}

void __attribute__((interrupt)) __cs3_isr_swi(void) {
	while(1);
}

void __attribute__((interrupt)) __cs3_isr_pabort(void) {
	while(1);
}

void __attribute__((interrupt)) __cs3_isr_dabort(void) {
	while(1);
}

void __attribute__((interrupt)) __cs3_isr_fiq(void) {
	while(1);
}

/**** Exported functions ****/

void config_interrupts(void) {
	disable_A9_interrupts();
	set_A9_IRQ_stack();
	config_GIC();
	// Configure interrupts on the I/O side
	for (int i = 0; i < num_interrupts; ++i) {
		if (interrupt_handlers[i].enable != NULL) interrupt_handlers[i].enable();
	}
	enable_A9_interrupts();
}

void config_interrupt(int interrupt_id, void (*enable)(void), void (*handler)(void)) {
	interrupt_handlers[num_interrupts].interrupt_id = interrupt_id;
	interrupt_handlers[num_interrupts].enable = enable;
	interrupt_handlers[num_interrupts].handler = handler;
	++num_interrupts;
}
