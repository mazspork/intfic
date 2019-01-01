// event.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// defines the activities used for actually playing the game

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ifs.h"

extern vocabulary vocab;
extern sentence_type sentence;
extern product products;
extern message messages;
extern int cmderr;


event* event::root = NULL;

// event constructor

event::event (char *s) {				// constructor: initialize all to NULL
	wordlist        = NULL;				// if zero, do not use in comparisons
	prod_conditions = NULL;				// same here
	obj_conditions  = NULL;				// and here
	message_text    = NULL;				// message to print if successful
	prod_changes    = NULL;				// products to affect if successful
	obj_changes     = NULL;				// objects to relocate if successful
	next            = NULL;				// link pointer to zero
	qlist           = NULL;          // assume no queries upon firing event

	indirect_message_text = NULL;		// assume the message is allocated within

	if (!(name = new char [strlen (s) + 1])) catastrophy ("1");

	strcpy (name, s);						// enter name of object

	if (!strcmp (name, "ROOT"))		// is this the root event?
		root = this;
	else {
		for (event *e = root; e->next; e = e->next);
		e->next = this;
		}

	}

// class event destructor

event::~event (void) {					// destructor: kill all dynamic vars
	prcond *a, *b;
	obcond *c, *d;
	wordnode *e, *f;

	if (!strcmp (name, "ROOT")) {		// deallocating root object?
		event *d, *e = root->next;
		while (e) {
			d = e;
			e = e->next;
			delete d;
			}
		}

	if (name) delete name;				// remove the name and the message

	if (message_text && !indirect_message_text) delete message_text;

	a = prod_conditions;             // deallocate the product conditions
	while (a) {
		b = a->next;
		if (a->errormsg)
			if (!a->indirect_errormsg) // only delete the error if it's allocated privately
				delete a->errormsg;
		delete a;
		a = b;
		}

	a = prod_changes;                // deallocate the list of product changes
	while (a) {
		b = a->next;
		delete a;
		a = b;
		}

	c = obj_conditions;              // deallocate the list of object conditions
	while (c) {
		d = c->next;
		if (c->errormsg)
			if (!c->indirect_errormsg)	// only deallocate error msg if it was privately allocated
				delete c->errormsg;
		delete c;
		c = d;
		}

	c = obj_changes;                 // deallocate the list of object changes
	while (c) {
		d = c->next;
		delete c;
		c = d;
		}

	e = wordlist;							// deallocate the list of words
	while (e) {
		f = e->next;
		delete e;
		e = f;
		}

	qnode *q, *r = qlist;				// deallocate query nodes
	while (r) {
		q = r->next;
		delete r->prompt;
		delete r;
		r = q;
		}

	}

// get name of this event

char *event::getname (void) {
	return name;
	}

// add a condition for a specific user entry
// this is a reentrant function in respect to the internal structure

void event::condition (char *s) {
	wordnode *w;
	int n, a = 0;

	while (s) {
		n = vocab.getnumber (getnext (s));
		if (n) {													// did the word exist?
			if (wordlist && vocab.getother (n) >= 0)	// is it a word with two meanings?
				n = vocab.getother (n);

			if (!wordlist) {                       	// but is it the first?
				if (!(w = wordlist = new wordnode)) catastrophy ("2");
				w->next = NULL;
				}
			else {
				if (!(w = new wordnode)) catastrophy ("3");
				w->next = wordlist->next;
				wordlist->next = w;
				}

			w->number = n;
			a++;
			}
		}

	if (!a) error ("No words matched in event", name, 0);

	}

// add a condition for relation between two objects

void event::condition (object *obj1, object_relation rel, object *obj2, char *err) {
	obcond *b;

	if (!obj_conditions) {
		if (!(b = obj_conditions = new obcond)) catastrophy ("4");
		b->next = NULL;
		}
	else {
		if (!(b = new obcond)) catastrophy ("5");
		b->next = obj_conditions->next;
		obj_conditions->next = b;
		}

	b->a        = obj1;
	b->b        = obj2;
	b->relation = rel;

	if (err) {
		if (!strncmp (err, "MSG", 3)) {
			b->errormsg = messages.getmsg (atoi (&err [3]));
			b->indirect_errormsg = 1;
			}
		else {
			if (!(b->errormsg = new char [strlen (err) + 1])) catastrophy ("6");
			strcpy (b->errormsg, err);
			b->indirect_errormsg = 0;
			}
		}
	else b->errormsg = NULL;
	}

// add an object change

void event::change (object *obj1, object_relation rel, object *obj2) {
	obcond *b;

	if (!obj_changes) {
		if (!(b = obj_changes = new obcond)) catastrophy ("7");
		b->next = NULL;
		}
	else {
		if (!(b = new obcond)) catastrophy ("8");
		b->next = obj_changes->next;
		obj_changes->next = b;
		}

	b->a        = obj1;
	b->b        = obj2;
	b->relation = rel;
	b->errormsg = NULL;					// no errors in change lists
	}

// add a product change

void event::change (int p, char c, int v) {
	prcond *a;

	if (!prod_changes) {
		if (!(a = prod_changes = new prcond)) catastrophy ("9");
		a->next = NULL;
		}
	else {
		if (!(a = new prcond)) catastrophy ("10");
		a->next = prod_changes->next;
		prod_changes->next = a;
		}

	a->product  = p;
	a->relation = c;
	a->value    = v;
	a->errormsg = NULL;
	}

// add a condition for a product of some previous event

void event::condition (int p, char c, int v, char *err) {
	prcond *a;

	if (!prod_conditions) {
		if (!(a = prod_conditions = new prcond)) catastrophy ("11");
		a->next = NULL;
		}
	else {
		if (!(a = new prcond)) catastrophy ("12");
		a->next = prod_conditions->next;
		prod_conditions->next = a;
		}

	a->product  = p;
	a->relation = c;
	a->value    = v;

	if (err) {
		if (!strncmp (err, "MSG", 3)) {
			a->errormsg = messages.getmsg (atoi (&err [3]));
			a->indirect_errormsg = 1;
			}
		else {
			if (!(a->errormsg = new char [strlen (err) + 1])) catastrophy ("13");
			strcpy (a->errormsg, err);
			a->indirect_errormsg = 0;
			}
		}
	else a->errormsg = NULL;
	}

// add a query to the list - the product number and the prompt string

void event::query (int i, char *s) {
	qnode *q = new qnode;
	q->prompt = new char [strlen (s) + 1];
	strcpy (q->prompt, s);
	q->product = i;

	if (qlist) {
		q->next = qlist->next;
		qlist->next = q;
		}
	else {
		qlist = q;
		q->next = NULL;
		}
	}

// ask questions and get inputs to products

void event::query (void) {
	const buffersize = 16;
	char b [buffersize], *c;
	int i;

	for (qnode *q = qlist; q; q = q->next) {
		printout (' ');
		for (c = q->prompt; *c; c++) printout (*c);
		while (!(i = input (b, buffersize - 1)));
		prettyprint ("\n");

		for (i = (*b > 0), c = b; *c; c++) {
			if (*c == '\n') {
				c = '\0';
				break;
				}
			if (!isdigit (*c) && *c != '\n') i = 0;
			}

		if (i)
			products.list [q->product] = atoi (b);
		else
			products.list [q->product] = -1;					// a deliberate error
		}

	}

// add a message to be printed when conditions meet

void event::set_message (char *m) {
	if (!strncmp (m, "MSG", 3)) {
		message_text = messages.getmsg (atoi (&m [3]));
		indirect_message_text = 1;
		}
	else {
		if (!(message_text = new char [strlen (m) + 1])) catastrophy ("14");
		strcpy (message_text, m);
		}
	}

// change the states of the event when everything is met
// returns 0 if no events fired.
// or -1 if an event generated an error

int event::affect (int when) {
	int i = 0;

	for (event *e = event::root; e; e = e->next)    // go through events
		if (e->match (when)) {
			i += e->fire ();        						// fire one if it matches
			if (when == AFTER_ENTRY) break;				// quit if a command succeeded
			}

	return i;
	}

// get the next object of current object from its linked list pointer

event *event::getnextevent (void) {
	return next;
	}

// check for match in event
// as it is now, it must match ALL words in the event.

int event::match (int when) {
	int j, i, k = 0;

	if ((!strcmp (name, "ROOT")) ||								// check for root object and
		(when == BEFORE_ENTRY && wordlist) ||         		// if the time of check doesn't
		(when == AFTER_ENTRY && !wordlist)) return 0;		// match with the type of event

	for (wordnode *w = wordlist; w; w = w->next) {        // go through wordlist of event
		j = 0;                        							// assume we don't find the word in the sentence

		switch (vocab.gettype (w->number)) {
			case WC_VERB:
				if (w->number == sentence.verb) j = 1;			// was the verb there?
				break;
			case WC_ACTOR:												// was the actor there?
				if (!k) {
					if (w->number == sentence.actor) j = k = 1;
					for (i = 0; sentence.preplist [i]; i++)		// preposition nouns?
						if (w->number == sentence.preplist [i])
							j = 1;
					for (i = 0; sentence.nounlist [i]; i++)		// go through nouns
						if (w->number == sentence.nounlist [i])
							j = 1;
					break;
					}														// if already an actor, go to noun/prep
			case WC_ADJECTIVE:
			case WC_NOUN:
				for (i = 0; sentence.preplist [i]; i++)		// preposition nouns?
					if (w->number == sentence.preplist [i])
						j = 1;
				for (i = 0; sentence.nounlist [i]; i++)		// go through nouns
					if (w->number == sentence.nounlist [i])
						j = 1;
				break;
			case WC_PREPOSITION:
				if (w->number == sentence.prep) j = 1;       // preposition?
				break;

			case WC_ADVERB:											// we don't handle these - just assume they're ok.
			case WC_ARTICLE:
				j = 1;
				break;

			default:														// just in case...
				break;
			}

		if (!j) return 0;
		}

	for (obcond *o = obj_conditions; o; o = o->next) {    // go through object conditions
		i = 0;                           						// assume failure to match

		switch (o->relation) {
			case OR_IN:
				if (o->a->parent->id == o->b->id) i = 1;
				break;
			case OR_WITH:
				if (o->a->issister (o->b)) i = 1;
				break;
			case OR_HIDDEN:
				if (o->a->parent == NULL) i = 1;
				break;
			case OR_WITHIN:
				if (o->a->parent->id == o->b->id || o->a->issister (o->b)) i = 1;
				break;
			case OR_NOTIN:
				if (o->a->parent->id != o->b->id) i = 1;
				break;
			case OR_NOTWITH:
				if (o->a->parent != o->b->parent) i = 1;
				break;
			case OR_NOTWITHIN:
				if (o->a->parent->id != o->b->id && !o->a->issister (o->b)) i = 1;
				break;
			case OR_WASIN:
				if (o->a->lastparent->id == o->b->id && o->a->flags.moved) i = 1;
				break;
			case OR_WASWITH:
				if (o->a->lastparent->id == o->b->parent->id && o->a->flags.moved) i = 1;
				break;
			case OR_WASHIDDEN:
				if (o->a->lastparent == NULL && o->a->flags.moved) i = 1;
				break;
			case OR_WASWITHIN:
				if ((o->a->lastparent->id == o->b->id || o->a->lastparent->id == o->b->parent->id) && o->a->flags.moved) i = 1;
				break;
			case OR_WASNOTIN:
				if (o->a->lastparent->id != o->b->id && o->a->flags.moved) i = 1;
				break;
			case OR_WASNOTWITH:
				if (o->a->lastparent->id != o->b->parent->id && o->a->flags.moved) i = 1;
				break;
			case OR_WASNOTWITHIN:
				if (o->a->lastparent->id != o->b->id && o->a->lastparent->id != o->b->parent->id && o->a->flags.moved) i = 1;
				break;

			default:
				break;
			}

		if (!i) {
			if (o->errormsg) {
				prettyprint (o->errormsg, "\n");
				cmderr = 1;
				}
			return 0;
			}
		}

	for (prcond *p = prod_conditions; p; p = p->next) {
		if (!products.match (p->product, p->relation, p->value)) {
			if (p->errormsg) {
				prettyprint (p->errormsg, "\n");
				cmderr = 1;
				}
			return 0;
			}
		}

	return 1;
	}

// "fire" an event

int event::fire (void) {
	query ();									// do queries for event

	if (message_text != NULL) prettyprint (message_text, "\n");

	object *c = NULL;							// avoid NULL ambiguity later

	for (obcond *o = obj_changes; o; o = o->next) {
		switch (o->relation) {
			case OR_IN:                   // move as child of other object
				o->a->setparent (o->b);
				break;
			case OR_WITH:						// move as sister of other object
				o->a->setsister (o->b);
				break;
			case OR_HIDDEN:					// move out of the way (no parent)
				o->a->setparent (c);
				break;
			default:
				break;
			}
		}

	for (prcond *p = prod_changes; p; p = p->next)
		products.affect (p->product, p->relation, p->value);

	return message_text ? 1 : 0;			// only beep later if message here
	}

