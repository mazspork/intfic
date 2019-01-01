// user.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// Handles input and output from the user.

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include "ifs.h"

// external definitions

extern vocabulary vocab;
extern sentence_type sentence;
extern product products;
extern message messages;
extern player_data pdata;
extern int screenwidth;

extern int
	ERROR,
	W_LOOK,
	W_INVENTORY,
	W_GO,
	W_QUIT,
	W_GET,
	W_PUT,
	W_HELP,
	W_AND,
	W_OR,
	W_ALL,
	W_IT,
	PLAYER_INIT;

// global data

int upper = 1;

// function definitions

// function getnext
//
// get next whitespace or zero-terminated string from p.
// p is a reference to the string pointer, so it will
// change the original pointer in the calling function.
// returns the address of the word found.

char *getnext (char *&p) {
	static char buffer [0x80];				// max identifier length for word
	char *q = buffer;

	while (isspace (*p)) p++;				// skip white space
	while (*p && !isspace (*p)) *q++ = mytoupper (*p++);
	*q = '\0';
	while (isspace (*p)) p++;				// skip white space
	if (!*p) p = NULL;

	return buffer;
	}

// function prettyprint
//
// Print string in such a way that words are not split across lines
// takes from 1 to 6 parameters, which are character strings to print.
// In this way, it is possible to prettyprint several characters in a
// single function call.
//
// There are now 4 variables to be used, namely:-
//
//   1) $TIME - the current time
//   2) $COUNTRY - the country where the player lives
//   3) $NAME - the player's name (full name incl. Mr/Ms)
//   4) $AGE - the player's age
//   5) $FNAME
//   6) $LNAME
//
// Note that these are always character strings and not numeric values.
//

void prettyprint (char *s, char *s1, char *s2, char *s3, char *s4, char *s5) {
	static unsigned char buffer [0x80], *ptr = buffer, spc = (unsigned char) ' ';
	static int c = 0;
	int i;
	char *p, qbuf [32], *q, ch;

	while (s) {

		if (*s != ',' && *s != '.' && s [strlen (s)] != '\n') {
			if (c >= screenwidth) {
				printout (buffer);
				c = 0;
				ptr = buffer;
				}
			else c++;
			*ptr++ = spc;
			*ptr   = 0;
			}

		while (*s) {
			i = 0;

			if (!strncmp (s, "$FNAME", 6)) {
				strcpy (qbuf, pdata.firstname);
				i = 6;
				}
			else if (!strncmp (s, "$LNAME", 6)) {
				strcpy (qbuf, pdata.lastname);
				i = 6;
				}
			else if (!strncmp (s, "$COUNTRY", 8)) {
				strcpy (qbuf, pdata.country);
				i = 8;
				}
			else if (!strncmp (s, "$AGE", 4)) {
				strcpy (qbuf, pdata.age);
				i = 4;
				}
			else if (!strncmp (s, "$TIME", 5)) {
				time_t secs_now = products.list [products.global_timer];
				struct tm *time_now = localtime (&secs_now);
				strftime (qbuf, 31, "%x %X", time_now);
				i = 5;
				}

			if (i) {
				s += i;
				i = strlen (qbuf);
				p = s;
				}
			else {
				q = qbuf, p = s;
				while (*p && *p != SPACE && *p != '\n') *q++ = *p++, i++;
				*q = 0;
				s += i;
				}

			if (i + c >= screenwidth - 1) {			// exeeding screen width?
				printout (buffer);
				c = 0;										// then flush buffer
				ptr = buffer;
				}

			c += i;

			for (q = qbuf; *q; q++) {
				ch = *q;
				if (upper && myisalnum (ch)) {
					upper = 0;
					*ptr++ = mytoupper (ch);
					}
				else *ptr++ = ch;
				if (ch == '.' || ch == '!' || ch == '?') upper = 1;
				}

			*ptr = 0;

			if (*p == '\n') {
				if (c) printout (buffer);
				ptr = buffer;								// indent if necessary
				*ptr++ = ' ';
				*ptr++ = ' ';
				*ptr   = 0;
				c = 2;
				s++;
				}
			if (*p == SPACE) {
				*ptr++ = ' ';
				*ptr   = 0;
				s++;
				c++;
				}
			}

		s  = s1;												// cycle the parameters
		s1 = s2;
		s2 = s3;
		s3 = s4;
		s4 = s5;
		s5 = NULL;
		}
	}

// function getsentence
//
// Reads a sentence from the user and places appropriate words in the
// sentence structure (global), according to word classes. The sentence
// might lie in the input buffer from a previous command with an AND-link.

void getsentence (int op) {
	const buffersize = 80;

	static char buffer [buffersize];					// possibly keep something here
	static char *r = 0;

	int h, i, j, k, l, t, ove, opr, oac, conti = 0;
	wordnode *n;
	char *p, *q, *s;

	if (!r || op) {										// new i/p if empty buf or prev error
		r = buffer;
		prettyprint ("\n");
		strcpy (r, messages.getmsg (47));
		while (*r) printout (*r++);
		printout (' ');

		ove = sentence.verb, sentence.verb = 0;
		opr = sentence.prep, oac = sentence.actor;
		sentence.prep = sentence.actor = 0;			// assume nothing there

		l = input (r = buffer, buffersize - 1);

		printout ('\n');
		printreset ();
		r = buffer;

		if (l == -1) {
			r = NULL;
			sentence.verb  = ove;
			sentence.prep  = opr;
			sentence.actor = oac;
			return;											// repeat the same sentence
			}

		if (!l) {
			r = NULL;               					// no input at all
			return;
			}
		}
	else {
		conti = 1;
		sentence.prep = 0;
		}

	for (q = r; *q; q++) {

		if (*q == ',') {									// a comma might mean AND...
			for (s = q; *s; s++);
			while (s >= q) {
				*(s+5) = *(s+1);
				s--;
				}
			*(q)   = ' ';									// so substitute
			*(q+1) = 'A';
			*(q+2) = 'N';
			*(q+3) = 'D';
			*(q+4) = ' ';
			}

		if (!myisalnum (*q))
			*q = ' ';
		else
			*q = mytoupper (*q);
		}

	for (h = j = k = l = 0; r && k < 7 && j < 7;) {
		p = getnext (r);
		i = vocab.getnumber (p);

		if (i == W_AND && l) break;
		if (i == W_IT) {
			while (sentence.nounlist [j]) j++;
			h = 1;
			}

		t = vocab.gettype (i);
		if (t == WC_ACTOR && sentence.verb) t = WC_NOUN;	// handle genitive case
		if (t == WC_VERB && sentence.verb && !conti) {
			t = WC_NOUN;
			i = vocab.getother (i);
			}

		switch (t) {
			case WC_ACTOR:
				if (!sentence.actor) sentence.actor = i;
				break;
			case WC_VERB:
				sentence.verb = i;									// otherwise insert new verb
				l = 1;
				conti = 0;
				break;
			case WC_NOUN:
			case WC_ADJECTIVE:
				if (sentence.prep)
					sentence.preplist [k++] = i;
				else {
					sentence.nounlist [j++] = i;
					h = 1;
					}
				l = 1;
				break;
			case WC_PREPOSITION:
				sentence.prep = i;
				l = 1;
				break;

			default:									// includes adverbs and articles
				break;
			}
		}

	if (!h && k)
		for (j = 0; j < k; j++)
			sentence.nounlist [j] = sentence.preplist [j];

	sentence.nounlist [j] = sentence.preplist [k] = 0;
	}

// function input
//
// standard input routine - adds characters to char* given,
// returns number of characters written. The parameters are
// where to put the input (a char pointer) and a maximum width
// for the input.

int input (char *r, int width) {
	int l = 0, looping = 1, counter1 = 0, counter2 = 0;
	unsigned char ch;

	while (looping > 0) {

		printout ('_');

		while (!kbhit ()) {

			if (counter1++ == 100) {
				animate ();						// graphics animation
				counter1 = 0;
				}

			if (counter2++ == 500) {		// cursor flash
				printout (8);
				printout (' ');
				}

			if (counter2 == 1000) {
				printout (8);
				printout ('_');
				counter2 = 0;
				}
			}

		printout (8);

		ch = getch ();
		if ((ch == '\n' || ch == NEWLINE) && l) break;

		if (ch == 8 && l) {							// Backspace
			printout (ch);
			l--;
			r--;
			}

		else if (!ch) {
			switch (getch ()) {						// put ext. keycodes here
				case F1:
					sentence.verb = W_HELP;
					*sentence.nounlist = 0;
					l = looping = 0;
					break;
				case F8:
					return -1;							// use previous sentence again
				case F9:
					sentence.verb = W_LOOK;
					*sentence.nounlist = 0;
					l = looping = 0;
					break;
				case F10:
					sentence.verb = W_INVENTORY;
					*sentence.nounlist = 0;
					l = looping = 0;
					break;
				case Del:
					while (l) {
						printout (8);
						r--;
						l--;
						}
				default:
					break;
				}
			}
		else if (ch > 31 && l < width) {
			l++;
			*r++ = ch;
			printout (ch);
			}
		}

	*r = '\0';

	return l;
	}
