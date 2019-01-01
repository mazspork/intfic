


#include <iostream.h>
#include <math.h>


void main (void) {

	double d;

	cout << "Enter double: "; cin >> d;

	cout << "\n" << d << "\n";

	int i = log10 (d) + 1;

	cout << "Digits to the left of the comma: " << i << "\n";

	double e = d - floor (d);

	cout << "Fraction = " << e;

	cout << "Log10 (fraction) is " << log10 (e) << "\n";




	}



