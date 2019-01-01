// interrup.cpp
//
// handles system interrupts for animating graphics etc.

#include <dos.h>

// constants

const CLOCK_INTERRUPT = 0x1C;

// globals

void interrupt cdecl (*oldvector) (...);

// functions

// clock tick interrupt handler - every 1/20th second

void interrupt handler (void) {

	// check for specific graphics to be animated...


//	if (room == INFIRMERY) {
//		...
//		...
//	...


	}

void intercept_clock_vector (void) {
	oldvector = getvect (CLOCK_INTERRUPT);
	setvect (CLOCK_INTERRUPT, handler);
	}

void free_clock_vector (void) {
	setvect (CLOCK_INTERRUPT, oldvector);
	}
