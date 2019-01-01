// IFS.cpp
//
// Interactive Fiction System - international version 1.1
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// Source language: AT&T C++ 2.0 with Borland libraries
// (no revision control)
//

// includes //

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include "ifs.h"

// globals //

vocabulary vocab;								// the vocabulary symbol tables
object root_object ("ROOT");				// the objects initialises with ROOT
event root_event ("ROOT");					// the events initialises with ROOT
object *player;								// must have the player object
product products;								// the product symbol tables
message messages;								// the message symbol tables
sentence_type sentence;						// the default sentence type
int cmderr;										// an error was printed by an event
void (*do_anim) (void) = stub_anim;		// pointer to animation function
player_data pdata;							// player's private data

const long
	SECOND =        1,                  // constants for time offsets
	MINUTE =       60,
	HOUR   =     3600,
	DAY    =    86400,
	MONTH  =  2629800,
	YEAR   = 31557600;

int
	animflag = 1,
	soundflag = 0,
	graphon = 0,
	ERROR = 0,
	W_LOOK = 0,
	W_INVENTORY =0,
	W_GO = 0,
	W_QUIT = 0,
	W_GET = 0,
	W_PUT = 0,
	W_HELP = 0,
	W_AND = 0,
	W_OR = 0,
	W_ALL = 0,
	W_IT = 0,
	W_WAIT = 0,
	W_GIVE = 0,
	W_WHAT = 0,
	W_YOU = 0,
	W_IN = 0,
	W_OUT = 0,
	PLAYER_INIT = 0;

int gdriver = -1;
int gmode;

// externs

extern void clrgraph (void);
extern void draw_assignment (void);
extern int ypos;
extern void titles (void);

// prototypes

void tellcore (void);

// main code - load the world, keep accepting inputs until end of game...
// sounds easy, doesn't it? Well, it's not. And don't peek around my code.

void main (int argc, char **argv) {
	object *o, *p, *actor;											// domestic object pointers
	int i, j, old_id, cont;
	int inperr = 0;
	char *who;
	if (!(player = new object ("PLAYER"))) catastrophy ("14");

	clrscr ();															// clear text area

	i = 1, j = 0;

	while (argv [i][0] == '/' || argv [i][0] == '-') {
		switch (tolower (argv [i] [1])) {
			case 'g': gdriver = atoi (argv [++i]), j = 1;
						 cout << "Graphics mode set to " << gdriver << endl;
						 break;
			case 'n': animflag = 0, j = 1;
						 cout << "Animation suspended" << endl;
						 break;
			case 's': soundflag = 1, j = 1;
						 cout << "Sound on" << endl;
						 break;
			}
		i++;
		}

	if (j) getch ();

	graphon = 1;

	if (!ERROR)
		if (!(ginit ())) {											// go to graphics mode
			cout << "Sorry -- this program needs a graphics adapter\n";
			ERROR = 1;
			gclose ();
			}

	titles ();

	loadworld (argv [i]);											// load the game !

	if (!ERROR) {														// if no errors during initialization, play the game

		draw_header ();
		draw_assignment ();

		player->setbrief (messages.getmsg (49));
		player->flags.takeable = 0;
		player->flags.actor    = 1;

		if (pdata.sex [0] == 'm') {										// if Ms. Player...
			o = object::search ("GIRLFRIEND");
			if (o) o->setparent (player->parent);
			}

		if (pdata.sex [0] == 'f') {										// if Mr. Player...
			o = object::search ("BOYFRIEND");
			if (o) o->setparent (player->parent);
			}

		products.list [products.global_timer] = YEAR * 20 + MONTH * 11;

		prettyprint ("\n");
		printreset ();                                     // reset text area

		player->parent->grload (1);                        // load graphics
		player->parent->grdraw ();                         // display graphics
		player->describe ();                               // describe scene

		object::ready ();												// get objects ready

		do {

			old_id = player->parent->id;							// remember where the player is

			if (event::affect (BEFORE_ENTRY)) {					// all non-entry events
				if (soundflag) {
					for (i = 1; i < 3; i++) {							// if one or more fired (with msg), make a noise
						sound (880);										// 880 Hz for 100 ms - twice!
						delay (100);
						nosound ();
						delay (100);
						}
					}
				}

			if (!products.list [products.game_running]) break;	// has the game ended?

			if (old_id != player->parent->id) {					// has the player dossed?
				player->parent->grload (1);
				player->parent->grdraw ();							// then get new graphic
				player->describe ();
				}

			player->checkmovements ();								// check movements around the player

			object::ready ();											// get objects ready

			do {
				products.elapse (MINUTE);							// tick timers

				getsentence (inperr);								// read user's input

				if (sentence.actor) {								// who's the actor?
					actor = object::search (sentence.actor, player);  // a computer-character?
					if (actor && actor->parent->id != player->parent->id) {
						prettyprint (actor->brief2, messages.getmsg (17), "\n");
						actor = NULL;
						}
					if (actor && !*sentence.nounlist && sentence.verb != W_INVENTORY && sentence.verb != W_WHAT) {
						prettyprint (messages.getmsg (19), actor->brief2, messages.getmsg (20), "\n");
						actor = NULL;
               	}
					}
				else actor = player;									// or the player him/herself?
				} while (!actor);

			who    = actor->brief2;
			inperr = 0;													// assume no errors
			old_id = player->parent->id;							// remember player's parent identifier
			cont   = (player->id == actor->id);					// for later contragation of verbs
			cmderr = 0;

			i = event::affect (AFTER_ENTRY);

			if (!cmderr && i <= 0 && player->parent->id == old_id) {	// if nothing fired on user entry or error, assume generic command

				o = object::search (sentence.nounlist, actor);

				if (sentence.verb == W_LOOK) {
					if (!*sentence.nounlist) {						// if it's generic (standalone)
						player->parent->grload (1);				// reload graphics on demand
						player->parent->grdraw ();					// draw them
						player->describe ();							// and describe the scene again
						}
					else {												// if object-specific LOOK command
						if (!o) {                              // does the object exist?
							prettyprint (messages.getmsg (39), "\n");
							inperr = 1;
							}
						else {
							if (!o->rel (OR_WITHINALL, actor)) { // is it by the actor?
								prettyprint (o->brief2, messages.getmsg (17), "\n");
								inperr = 1;
								}
							else {
								if (sentence.prep == W_IN)	{		// looking inside something?
									if (o->flags.actor) {
										prettyprint (who, messages.getmsg (4), messages.getmsg (46), messages.getmsg (41), o->brief2, ".\n");
										inperr = 1;
										}
									else {
										prettyprint (messages.getmsg (41), o->brief2, who, messages.getmsg (10));
										o->listchildren ();
										}
									}
								else {
									if (o->grload (0)) o->grdraw ();
									prettyprint (o->verbose, "\n");
									}
								}
							}
						}
					}
				else if (sentence.verb == W_INVENTORY || sentence.verb == W_WHAT) {		// an inventory command
					prettyprint (actor->brief2, messages.getmsg (cont ? 31 : 28));			// who?
					actor->listchildren ();
					}
				else if (sentence.verb == W_GET) {				// a get command

					if (!o) {
						prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (10), messages.getmsg (33), ".\n");
						inperr = 1;
						}
					else {
						if (sentence.prep == W_OUT) {
							if (!(p = object::search (sentence.preplist, actor))) {
								inperr = 1;
								prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (10), messages.getmsg (33), ".\n");
								}
							else {
								if (!p->rel (OR_WITHIN, actor)) {
									prettyprint (p->brief2, messages.getmsg (17), "\n");
									inperr = 1;
									}
								else if (!o->rel (OR_IN, p)) {
									prettyprint (o->brief2, messages.getmsg (15), messages.getmsg (41), p->brief2, ".\n");
									inperr = 1;
									}
								else {
									o->setparent (actor);
									prettyprint (who, messages.getmsg (cont ? 2 : 23), o->brief, messages.getmsg (1));
									prettyprint (p->brief, ".\n");
									}
								}
							}
						else {
							if (o->rel (OR_WITH, actor)) {
								if (!o->flags.takeable) {
									prettyprint (who, messages.getmsg (4), messages.getmsg (2), o->brief2, ".\n");
									inperr = 1;
									}
								else {
									o->setparent (actor);
									prettyprint (who, messages.getmsg (cont ? 2 : 23), o->brief2, ".\n");
									}
								}
							else if (o->rel (OR_IN, actor)) {
								prettyprint (who, messages.getmsg (13), messages.getmsg (cont ? 31 : 28), o->brief2, ".\n");
								inperr = 1;
								}
							else {
								prettyprint (o->brief2, messages.getmsg (17), "\n");
								inperr = 1;
								}
							}
						}
					}
				else if (sentence.verb == W_PUT)	{				// a put command
					if (!o) {
						prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (10), messages.getmsg (33), ".\n");
						inperr = 1;
						}
					else {
						if (sentence.prep == W_IN) {
							if (!(p = object::search (sentence.preplist, actor))) {
								prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (10), messages.getmsg (33), ".\n");
								inperr = 1;
								}
							else {
								if (!p->rel (OR_WITHIN, actor)) {
									prettyprint (p->brief2, messages.getmsg (17), "\n");
									inperr = 1;
									}
								else if (!o->rel (OR_WITHIN, actor)) {
									prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (31), o->brief2, ".\n");
									inperr = 1;
									}
								else {
									prettyprint (who, messages.getmsg (cont ? 5 : 25), o->brief, messages.getmsg (41), p->brief, ".\n");
									o->setparent (p);
									}
								}
							}
						else {
							if (!o->rel (OR_IN, actor)) {
								prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (31), o->brief2, ".\n");
								inperr = 1;
								}
							else {
								o->setparent (actor->parent);
								prettyprint (who, messages.getmsg (cont ? 5 : 25), o->brief2, messages.getmsg (43));
								prettyprint (messages.getmsg (44), actor->parent->brief2, ".\n");
								}
							}
						}
					}
				else if (sentence.verb == W_GIVE) {					// a give command

					if (*sentence.nounlist == W_YOU) {				// check for "give me the thing" reversed sentence
						for (int i = 1; sentence.nounlist [i]; i++)
							sentence.nounlist [i - 1] = sentence.preplist [i];
						sentence.nounlist [i] = 0;
						sentence.preplist [0] = W_YOU;
						sentence.prep = 1;
						}

					if (!sentence.prep) {
						prettyprint (messages.getmsg (42), "\n");
						inperr = 1;
						}
					else {
						if (!(p = object::search (sentence.preplist, actor))) { // to whom?
							prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (34), "\n");
							inperr = 1;
							}
						else {
							if (!o || !o->flags.takeable) {
								prettyprint (who, messages.getmsg (4), messages.getmsg (26), messages.getmsg (33));
								prettyprint (messages.getmsg (32), p->brief2, ".\n");
								inperr = 1;
								}
							else {
								if (!o->rel (OR_WITHIN, actor)) {
									prettyprint (who, messages.getmsg (cont ? 29 : 30), messages.getmsg (31), o->brief2, ".\n");
									inperr = 1;
									}
								else {
									if (!p->rel (OR_WITH, actor)) {
										prettyprint (p->brief2, messages.getmsg (17), "\n");
										inperr = 1;
										}
									else {
										if (!p->flags.actor) {
											prettyprint (who, messages.getmsg (4), messages.getmsg (26), messages.getmsg (35), messages.getmsg (32));
											prettyprint (p->brief2, ".\n");
											inperr = 1;
											}
										else {
											o->setparent (p);
											prettyprint (who, messages.getmsg (cont ? 26 : 27), o->brief2, messages.getmsg (32), p->brief2, ".\n");
											}
										}
									}
								}
							}
						}
               }
				else if (sentence.verb == W_GO) {				// a go command
					if (!o) {
						prettyprint (who, messages.getmsg (12), "\n");
						inperr = 1;
						}
					else {
						if (actor->parent->id == o->id) {
							prettyprint (who, messages.getmsg (cont ? 21 : 22), messages.getmsg (13), o->brief, ".\n");
                  	}
						else if (o->flags.takeable || o->flags.actor) {
							prettyprint (who, messages.getmsg (48), o->brief2, ".\n");
                     inperr = 1;
							} 
						else if (!actor->parent->rel (OR_WITHNOPARENT, o)) {  // is o exit from actor's parent?
							prettyprint (who, messages.getmsg (40), o->brief2, messages.getmsg (1), actor->parent->brief2, ".\n");
							inperr = 1;
							}
						else {
							actor->setparent (o);
							}
						}
					}
				else if (sentence.verb == W_WAIT) {
					prettyprint (messages.getmsg (16), "\n");
            	}
				else if (sentence.verb == W_HELP) {
					clrgraph ();
					ypos = 0;
					prettyprint (messages.getmsg (36), "\n");
					tellcore ();
					getch ();
					printreset ();
					player->parent->grload (1);				// reload graphics on demand
					player->parent->grdraw ();					// draw them
					}
				else if (sentence.verb == W_QUIT) {
					products.list [products.game_running] = 0;
					}
				else if (sentence.verb == ERROR) {				// a bad verb
					prettyprint (messages.getmsg (7), "\n");
					inperr = 1;
					}
				else if (sentence.verb != W_QUIT && !i) {		// a good verb, but no event fired through
					prettyprint (who, messages.getmsg (8), "\n");
					inperr = 1;											// only if already no error from an event
					}
				}

			if (player->parent->id != old_id) {					// did the player move?
				player->parent->grload (1);
				player->parent->grdraw ();							// then get new graphic
				player->describe ();
				}

			} while (products.list [products.game_running]);	// until he types QUIT

		prettyprint (messages.getmsg (9), "\n.");				// farewell message
		getch ();														// wait for keyress
		gclose ();														// shut down graphics system
		}

	else cout << "Correct errors and try again.\n";			// if translation error
	}

// This function handles internal errors such as memory allocation failures

void catastrophy (char *s) {
	cout << "\nInternal IFS error: " << s << "\n";
	getch ();
	}

void tellcore (void) {
	long l1 = coreleft ();
	char l2 [16];
	prettyprint ("\nFree memory:", ltoa (l1, l2, 10), " bytes.\n");
	}

// redefinitions of some is... and to... functions


char mytolower (char c) {
	switch ((unsigned char) c) {
		case 142: return 132;
		case 143: return 134;
		case 144: return 130;
		case 146: return 145;
		case 153: return 148;
		case 154: return 129;
		case 157: return 155;
		case 165: return 164;
		case 181: return 160;
		case 182: return 131;
		case 183: return 133;
		case 199: return 198;
		case 210: return 136;
		case 211: return 137;
		case 212: return 138;
		case 224: return 162;
		case 226: return 147;
		case 229: return 128;
		case 128: return 135;
		}

	if (c >= 'A' && c <= 'Z') return (c + 32);

	return c;
	}

char mytoupper (char c) {
	switch ((unsigned char) c) {
		case 132: return 142;
		case 134: return 143;
		case 130: return 144;
		case 145: return 146;
		case 148: return 153;
		case 129: return 154;
		case 155: return 157;
		case 164: return 165;
		case 160: return 181;
		case 131: return 182;
		case 133: return 183;
		case 198: return 199;
		case 136: return 210;
		case 137: return 211;
		case 138: return 212;
		case 162: return 224;
		case 147: return 226;
		case 128: return 229;
		case 135: return 128;
		}

	if (c >= 'a' && c <= 'z') return (c - 32);

	return c;
	}

int myisascii (char c) {
	if (isascii (c) || (unsigned char) c > 127) return 1;

	return 0;
	}

int myisalnum (char c) {
	if (isalnum (c) || (unsigned char) c > 127) return 1;

	return 0;
	}

