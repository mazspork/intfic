#include <iostream.h>
#include <string.h>
#include <ctype.h>

void main (void) {

	char *a = "SVEAV�GEN";

	char *b = a;

	while (*b) {
		*b = tolower (*b);
		b++;
		}

	cout << a << "\n";

	}