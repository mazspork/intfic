// trans.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// Translates definition file into internal structures.

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ifs.h"

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
	W_WAIT,
	W_GIVE,
	W_WHAT,
	W_YOU,
	W_IN,
	W_OUT,
	PLAYER_INIT;

extern vocabulary vocab;				// the vocabulary defined elsewhere
extern product products;				// the product symbol table defined elsewhere
extern message messages;				// the messages symbol table defined elsewhere

char *curname;

// loadworld (filename of definition file)
//
// load a definition file
// this function will read from the file and construct all data structures
// and initialize them. This includes:
//
//  -the objects to define the world,
//  -the vocabulary used to manipulate them,
//  -the events that can happen, when and how and under which circumstances,
//  -the products resulting from actions,
//  -the messages to be used in the national language
//  -some basic system variables
//
// this function is truely one of the most complex with respect to nested
// if/then structures that I have ever written! Some nice grammar and
// syntax directed interpretation would have done much nicer!

void loadworld (char *filename) {
	const stkdepth     = 8;
	wordclass substate = NO_CLASS;

	enum {						// Main State Vector:
		INVALID,					//   illegal definition state
		OBJECTS,					//   defining objects
		VOCABULARY,				//   defining vocabulary
		PRODUCTS,				//   defining products
		EVENTS,					//   defining activities
		MESSAGES,				//   defining messages
		SYSTEM					//   defining system vars
		} state = INVALID;

	enum {						// Secondary State Vector
		S1_INVALID,				//   illegal substate
		CONDITIONS,				//   defining conditions for events
		ACTIONS					//   defining actions for events
		} substate1;


	char *mustdef = "You must define the system variable",
		  *nodef   = "keyword hasn't been defined in the vocabulary";

	char c, *q, *p, *ident, *temp, *pp, *buffer = new char [BUFFERSIZE];
	char ident2 [0x40];
	char filenames [stkdepth + 1] [11];
	object_relation substate2;
	object *o, *r, *o1, *o2;
	event *e;
	ifstream file [stkdepth];
	int lnum, lnums [stkdepth], i, j, oldstate = 0;
	int stkpos = 0;

	if (!buffer) catastrophy ("21");

	for (int pass = 0; pass < 2; pass++) {		// two passes
		lnum = 0;

		if (pass) {
			vocab.initialize ();						// now we know # words
			products.initialize ();					// and # of products
			messages.initialize ();					// and # of messages
			}

		file [stkpos].open (filename);			// open script file

		if (!file [stkpos]) {						// is it there?
			cout << "Could not open " << filename << " for input\n";
			exit (10);
			}

		strcpy (filenames [stkpos], filename);
		curname = filenames [stkpos];

		for (;;) {

			if (file [stkpos].eof ()) {				// end of file?
				file [stkpos].close ();					// then close it.

				if (stkpos) {				 				// was this an "included" file?
					stkpos--;								// then go up a level
					lnum = lnums [stkpos];
					curname = filenames [stkpos];
					}
				else
					break;	                  	   // else quit the pass
				}

			lnum++;

			file [stkpos].get (buffer, BUFFERSIZE);			// get until newline
			file [stkpos].get (c);									// eat the newline too
			p = buffer;

			while (isspace (*p)) p++;								// eat whitespace
			for (q = p; *q; q++) if (*q == ';') *q = 0;		// remove any comments

			if (oldstate != state) {								// dot when reading new section
//				cout << '.';
				oldstate = state;
				}

			if (*p) {													// if it wasn't a comment
				ident = getnext (p);									// remember identifier

				// are we including another file?

				if (!strcmp (ident, "INCLUDE")) {
					if (stkpos == stkdepth - 1) {
						cout << "Fatal: too many nested files.\n";
						exit (10);
						}

					lnums [stkpos] = lnum;
					q = getnext (p);

					file [++stkpos].open (q);

					if (!file [stkpos]) {
						cout << "Fatal: Could not open include file for input in line " << lnum << " of file " << curname << "\n";
						exit (10);
						}

					lnum = 0;
					curname = strcpy (filenames [stkpos], q);
					}

				// are we about to change the modus of the definitions?

				else if (!strcmp (ident, "OBJECTS"))
					state = OBJECTS;
				else if (!strcmp (ident, "VOCABULARY"))
					state = VOCABULARY;
				else if (!strcmp (ident, "PRODUCTS"))
					state = PRODUCTS;
				else if (!strcmp (ident, "EVENTS"))
					state = EVENTS;
				else if (!strcmp (ident, "MESSAGES"))
					state = MESSAGES;
				else if (!strcmp (ident, "SYSTEM"))
					state = SYSTEM;

				// was there a definition in an invalid modus?

				else if (state == INVALID) {
					if (pass) error ("Misplaced keyword", ident, lnum);
					}

				else if (state == MESSAGES) {
					if (!pass) messages.addcount (atoi (&ident [3]));
					else {
						if (!strncmp (ident, "MSG", 3))
							messages.entry (atoi (&ident [3]), p);
						else
							error ("Wrong MESSAGE keyword", ident, lnum);
						}
            	}

				// in case the current modus is vocabulary definitions, do a word

				else if (state == VOCABULARY) {
					if (!pass) {							// count word at pass one
						if (!strcmp (ident, "VERBS"))
							substate = WC_VERB;
						else if (!strcmp (ident, "NOUNS"))
							substate = WC_NOUN;
						else if (!strcmp (ident, "ADJECTIVES"))
							substate = WC_ADJECTIVE;
						else if (!strcmp (ident, "PREPOSITIONS"))
							substate = WC_PREPOSITION;
						else if (!strcmp (ident, "ARTICLES"))
							substate = WC_ARTICLE;
						else if (!strcmp (ident, "ADVERBS"))
							substate = WC_ADVERB;
						else if (!strcmp (ident, "ACTORS"))
							substate = WC_ACTOR;
						else {
							vocab.addwordcount ();
							while (p) {
								vocab.addwordcount ();
								(void) getnext (p);
								}
							}
						}
					else {
						if (!strcmp (ident, "VERBS"))
							substate = WC_VERB;
						else if (!strcmp (ident, "NOUNS"))
							substate = WC_NOUN;
						else if (!strcmp (ident, "ADJECTIVES"))
							substate = WC_ADJECTIVE;
						else if (!strcmp (ident, "PREPOSITIONS"))
							substate = WC_PREPOSITION;
						else if (!strcmp (ident, "ARTICLES"))
							substate = WC_ARTICLE;
						else if (!strcmp (ident, "ADVERBS"))
							substate = WC_ADVERB;
						else if (!strcmp (ident, "ACTORS"))
							substate = WC_ACTOR;
						else {
							vocab.enter (ident, substate);
							while (p) vocab.synonym (getnext (p));
							}
						}
					}

				// in case the current modus is object definitions, define one

				else if (state == OBJECTS) {
					if (pass) {
						if (!strcmp (ident, "OBJECT")) o = object::search (getnext (p));
						else if (!strcmp (ident, "PARENT")) {
							strcpy (ident2, getnext (p));
							if (!o->setparent (ident2))
								error ("Couldn't set PARENT object to", ident2, lnum);
							}
						else if (!strcmp (ident, "EXITS")) {
							while (p) {
								strcpy (ident2, getnext (p));
								if (!o->setexit (ident2))
									error ("Couldn't set EXIT object to", ident2, lnum);
								}
                  	}
						else if (!strcmp (ident, "BRIEF1")) {
							if (!(o->setbrief (p))) error ("Bad BRIEF1 description", p, lnum);
							}
						else if (!strcmp (ident, "BRIEF2")) {
							if (!(o->setbrief2 (p))) error ("Bad BRIEF2 description", p, lnum);
							}
						else if (!strcmp (ident, "VERBOSE")) o->setverbose (p);
						else if (!strcmp (ident, "GRAPHIC")) o->setgrfile (p);
						else error ("Wrong OBJECTS keyword", ident, lnum);
						}
					else {
						if (!strcmp (ident, "OBJECT")) new object (p);
						}
					}

				// in case the modus is product definitions, do the following

				else if (state == PRODUCTS) {
					if (!pass) {
						if (!strcmp (ident, "PRODUCT")) products.addcount (getnext (p));
						}
					else {
						if (!strcmp (ident, "PRODUCT")) {
							strcpy (ident2, getnext (p));
							if (strcmp (getnext (p), "=")) error ("You must use the equals sign here (without spaces)","",lnum);
							products.entry (ident2, atoi (getnext (p)));
							}
						}
					}

				// in case the modus is event definitions, take an event

				else if (state == EVENTS) {
					if (pass) {						// vocabulary & messages must be defined
						if (!strcmp (ident, "EVENT"))
							e = new event (p);
						else if (!strcmp (ident, "CONDITIONS"))
							substate1 = CONDITIONS;
						else if (!strcmp (ident, "ACTIONS"))
							substate1 = ACTIONS;
						else if (substate1 == CONDITIONS) {
							if (!strcmp (ident, "ENTRY")) {
								e->condition (p);
								}
							else if (!strcmp (ident, "OBJECT")) {
								temp = getnext (p);
								if (!(o1 = object::search (temp)))
									error ("No match for object ", temp, lnum);

								temp = getnext (p);
								if (!strcmp (temp, "IN"))
									substate2 = OR_IN;
								else if (!strcmp (temp, "WITH"))
									substate2 = OR_WITH;
								else if (!strcmp (temp, "WITHIN"))
									substate2 = OR_WITHIN;
								else if (!strcmp (temp, "NOTIN"))
									substate2 = OR_NOTIN;
								else if (!strcmp (temp, "NOTWITH"))
									substate2 = OR_NOTWITH;
								else if (!strcmp (temp, "NOTWITHIN"))
									substate2 = OR_NOTWITHIN;
								else if (!strcmp (temp, "WASIN"))
									substate2 = OR_WASIN;
								else if (!strcmp (temp, "WASWITH"))
									substate2 = OR_WASWITH;
								else if (!strcmp (temp, "WASWITHIN"))
									substate2 = OR_WASWITHIN;
								else if (!strcmp (temp, "WASNOTIN"))
									substate2 = OR_WASNOTIN;
								else if (!strcmp (temp, "WASNOTWITH"))
									substate2 = OR_WASNOTWITH;
								else if (!strcmp (temp, "WASNOTWITHIN"))
									substate2 = OR_WASNOTWITHIN;
								else {
									substate2 = OR_ERROR;
									error ("Wrong relation in OBJECT conditions in event", o1->getname (), lnum);
									}

								temp = getnext (p);
								if (!(o2 = object::search (temp)))
									error ("No match for object ", temp, lnum);

								if (!strcmp (getnext (p), "ERROR"))
									temp = p;
								else
									temp = NULL;

								if (substate2 != OR_ERROR)
									e->condition (o1, substate2, o2, temp);
								}
							else if (!strcmp (ident, "PRODUCT")) {
								if ((i = products.getindex (getnext (p))) == NOTFOUND)
									error ("Product not defined in condition","",lnum);
								else {
									q = getnext (p);
									c = *q;
									j = atoi (getnext (p));
									if (!strcmp (getnext (p), "ERROR"))
										temp = p;
									else
										temp = NULL;

									e->condition (i, c, j, temp);
									}
								}
							else error ("Wrong CONDITIONS keyword", ident, lnum);
							}
						else if (substate1 == ACTIONS) {
							if (!strcmp (ident, "MESSAGE")) {
								e->set_message (p);
								}
							else if (!strcmp (ident, "OBJECT")) {
								temp = getnext (p);
								if (!(o1 = object::search (temp)))
									error ("No match for object ", temp, lnum);

								temp = getnext (p);
								if (!strcmp (temp, "IN"))
									substate2 = OR_IN;
								else if (!strcmp (temp, "WITH"))
									substate2 = OR_WITH;
								else if (!strcmp (temp, "HIDE"))
									substate2 = OR_HIDDEN;
								else {
									substate2 = OR_ERROR;
									error ("Use IN, WITH or HIDE in OBJECT changes in object", o1->getname (), lnum);
									}

								if (substate2 == OR_HIDDEN)
									o2 = NULL;
								else {
									temp = getnext (p);
									if (!(o2 = object::search (temp)))
										error ("No match for object ", temp, lnum);
									}

								if (substate2 != OR_ERROR) e->change (o1, substate2, o2);
								}
							else if (!strcmp (ident, "PRODUCT")) {
								i = products.getindex (getnext (p));
								q = getnext (p);
								c = *q;
								j = atoi (getnext (p));
								e->change (i, c, j);
								}
							else if (!strcmp (ident, "INPUT")) {
								i = products.getindex (getnext (p));
								e->query (i, p);
								}
							else error ("Wrong ACTIONS keyword", ident, lnum);
							}
						}
					}

				// in case we're dealing with system variables

				else if (state == SYSTEM) {
					if (pass) {

						strcpy (ident2, ident);
						pp = getnext (p);

						if (!strcmp (ident2, "LOOK")) {
							if (!(W_LOOK = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "INVENTORY")) {
							if (!(W_INVENTORY = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "GO")) {
							if (!(W_GO = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "QUIT")) {
							if (!(W_QUIT = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "HELP")) {
							if (!(W_HELP = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "GET")) {
							if (!(W_GET = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "PUT")) {
							if (!(W_PUT = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "AND")) {
							if (!(W_AND = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "OR")) {
							if (!(W_OR = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "ALL")) {
							if (!(W_ALL = vocab.getnumber (pp)))
								error ("ALL", nodef, lnum);
							}
						else if (!strcmp (ident2, "IT")) {
							if (!(W_IT = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "WAIT")) {
							if (!(W_WAIT = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "GIVE")) {
							if (!(W_GIVE = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "WHAT")) {
							if (!(W_WHAT = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "YOU")) {
							if (!(W_YOU = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "IN")) {
							if (!(W_IN = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "OUT")) {
							if (!(W_OUT = vocab.getnumber (pp)))
								error (pp, nodef, lnum);
							}
						else if (!strcmp (ident2, "INITIAL")) {
							o = object::search ("PLAYER");

							if (!o->setparent (pp))
								error ("INITIAL location couldn't be found", "", lnum);
							else
								PLAYER_INIT = 1;
							}

						else error ("Wrong SYSTEM keyword", ident, lnum);
						}
					}
				}
			}
		}

	delete buffer;

	if (!PLAYER_INIT) error ("You must define an INITIAL location for the PLAYER object", "", 0);
	if (!W_GO)        error (mustdef, "GO", 0);
	if (!W_INVENTORY) error (mustdef, "INVENTORY", 0);
	if (!W_LOOK)      error (mustdef, "LOOK", 0);
	if (!W_QUIT)      error (mustdef, "QUIT", 0);
	if (!W_GET)       error (mustdef, "GET", 0);
	if (!W_PUT)       error (mustdef, "PUT", 0);
	if (!W_HELP)      error (mustdef, "HELP", 0);
	if (!W_AND)       error (mustdef, "AND", 0);
	if (!W_OR)        error (mustdef, "OR", 0);
	if (!W_ALL)       error (mustdef, "ALL", 0);
	if (!W_IT)        error (mustdef, "IT", 0);
	if (!W_WAIT)      error (mustdef, "WAIT", 0);
	if (!W_GIVE)      error (mustdef, "GIVE", 0);
	if (!W_WHAT)      error (mustdef, "WHAT", 0);
	if (!W_YOU)       error (mustdef, "YOU", 0);
	if (!W_IN)        error (mustdef, "IN", 0);
	if (!W_OUT)       error (mustdef, "OUT", 0);
	}

// error handler when encountering arbitrary information
// sets global ERROR to true.

void error (char *s, char *x, int l) {

	gclose ();

	cout << "\0x0D" << "Error: " << s;
	if (*x) cout << " " << x;
	if (l) cout << " at line " << l;
	cout << " in file " << curname << "\n";

	ERROR = 1;
   }
