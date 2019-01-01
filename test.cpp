#include <stdio.h>

void main (void) {
	int c;

	while ((c = getch ()) != 3) printf ("Struck key %d\n", c);
	}
