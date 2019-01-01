


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class bignum {
	char* v;
	int length;
public:
	bignum (long = 0);
	bignum (char*);
	~bignum ();
	void print (FILE* = stdout);
	};

bignum::bignum (long l) {
	static char temp [32];
	ltoa (l, temp, 10);
	bignum (temp);
	}

bignum::bignum (char* s) {
	length = strlen (s);
	v = new char [length + 1];
	strcpy (v, s);
	};

bignum::~bignum () {
	delete v;
	}

void bignum::print (FILE* fp) {
	fprintf (fp, "%s\n", v);
	}

void main (void) {
	bignum a = 12372323;
	bignum b = "876437683426879423876423876";
	a.print ();
	b.print ();
	}









