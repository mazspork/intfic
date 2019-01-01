// object.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// Defines the data structure and methods for manipulating
// The World, defined by interconnected objects.

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "ifs.h"


// initialisations of static data in object structure

object* object::root = NULL;
pic_t object::picture = { 0, 0 };
int object::count = 0;

// external declarations

extern vocabulary vocab;				// vocabulary must be known to objects
extern message messages;				// message symbol table must be known

extern void (*do_anim) (void);		// function to call when animating scenes

extern int animflag;

// prototypes for animations functions

void animate_gate (void);
void animate_airport (void);
void animate_checkin (void);
void animate_plane (void);
void animate_car (void);
void animate_departure (void);
void animate_airfield (void);
void animate_taxfree (void);
void animate_fieldoff (void);
void animate_locusts (void);

// class object constructor
// object must be allocated dynamically - except a "ROOT" object.

object::object (char *n) {
	name = brief = brief2 = verbose = grfile = NULL;
	parent = sister = child = next  = NULL;
	exitlist = NULL;
	wordlist = NULL;
	flags.moved    = 0;
	flags.takeable = 1;              // assume one can "take" the object
	flags.actor    = 0;					// assume it's not an actor object

	setname (getnext (n));           // set the name of the object (redundant at run time)

	char *t = getnext (n);

	if (!strcmp (t, "STATIC"))
		flags.takeable = 0;  			// object can't move parent
	else if (!strcmp (t, "ACTOR")) {
		flags.actor    = 1;				// object is an actor
		flags.takeable = 0;
		}

	if (!strcmp (name, "ROOT")) {		// Use first and only first
		root = this;						// the static pointer root is now this
		count = 0;                    // ROOT has identifier no. zero
		picture.list = NULL;          // ROOT hasn't any graphical information
		picture.size = NULL;
		}
	else {
		next = root->next;            // link this object in between root and the next one
		root->next = this;
		}

	id = count++;                    // assign and update unique identifier counter
	}

// class object destructor

object::~object (void) {
	object *o, *p;

	if (!strcmp (name, "ROOT")) {
		if (picture.list != NULL) delete picture.list;
		p = root->next;
		while (p) {
			o = p;
			p = p->next;
			delete o;								// implied recursive call to destructor
			}
		}

	wordnode *v, *w = wordlist;				// remove list of word identifiers
	while (w) {
		v = w;
		w = w->next;
		delete v;
		}

	objnode *m, *n = exitlist;					// remove list of exit objects
	while (n) {
		m = n;
		n = n->next;
		delete m;
		}

	if (name)    delete name;              // remove the strings (if allocated)
	if (brief2 && brief != brief2) delete brief2;
	if (brief)  delete brief;
	if (verbose) delete verbose;
	};

// set name of an object

void object::setname (char *s) {
	strupr (s);									// make sure that name is uppercase
	if (!(name = new char [strlen (s) + 1])) catastrophy ("26");
	strcpy (name, s);
	}

// set the brief description, and also the list of word IDs used
// this is the "the-form" (bestemt) of the brief description

int object::setbrief (char *p) {
	int num, typ, tot = 0;

	if (!(brief = new char [strlen (p) + 1])) catastrophy ("27");
	strcpy (brief, p);
	brief2 = brief;								// just to make sure

	for (char *q = p; *q; q++)					// remove punctuation for the checking
		if (!myisalnum (*q)) *q = ' ';

	wordnode *w = NULL;

	while (p) {
		num = vocab.getnumber (getnext (p));
		typ = vocab.gettype (num);

		if (typ == WC_VERB && vocab.getother (num) >= 0)
			num = vocab.getother (num);		// if it's a polyvalent word

		if (num) {
			tot++;
			if (w == NULL) {
				if (!(wordlist = w = new wordnode)) catastrophy ("29");
				}
			else {
				if (!(w->next = new wordnode)) catastrophy ("15");
				w = w->next;
				}
			w->number = num;
			}
		}

	if (tot)
		w->next = NULL;					// last in list
	else {
		error ("No words matched from vocabulary in object ", name, 0);
		return 0;
		}

	return 1;
	}

// set the "a-form" of the brief description

int object::setbrief2 (char *p) {
	if (!(brief2 = new char [strlen (p) + 1])) catastrophy ("38");
	strcpy (brief2, p);

   return 1;
	}

// set child pointer to given object address

void object::setchild (object *o) {
	child = o;
	}

// set graphics filename to s

void object::setgrfile (char *s) {
	if (!(grfile = new char [strlen (s) + 1])) catastrophy ("16");
	strcpy (grfile, s);
	}

// set an exit object by extending the linked object node list

int object::setexit (char *s) {
	object *o = search (s);
	objnode *n;

	if (o) {
		if (!(n = new objnode)) catastrophy ("28");
		n->ptr = o;

		if (exitlist) {						// if already one there
			n->next = exitlist->next;		// link into present list
			exitlist->next = n;
			}
		else {									// if none there already
			exitlist = n;						// make it the present
			n->next = NULL;
			}
		}
	else {
		error ("Could not set the EXIT object ", name, 0);
		return 0;
		}

	return 1;
	}

// is a certain object a valid exit from the current one?

int object::isexit (object *o) {
	for (objnode *n = exitlist; n; n = n->next)	// search through exit-list
		if (n->ptr->id == o->id) return 1;			// return true if match

	return 0;												// else return false
	}

// set sister pointer to given object address (run-time version)
// also affects parent pointer

void object::setsister (object *o) {
	object *q, *p = parent;

	if (p) {
		p = p->child;                             // get it out of current scope
		if (p->id == id)									// is it me?
			parent->child = sister;
		else {
			while (p->sister->id != id) p = p->sister;
			p->sister = sister;
			}
		}

	lastparent  = parent;						// remember the "old" parent object's address
	flags.moved = 1;								// signal that object has moved

	sister = o->sister;
	parent = o->parent;
	o->sister = this;
	}

// set sister pointer to given object (load-time version)
// returns TRUE/FALSE depending on whether sister object was found

int object::setsister (char *s) {
	object *q, *p, *o = search (s);

	if (o) {
		sister = o->sister;
		parent = o->parent;
		o->sister = this;
		}
	else {
		error ("Could not set SISTER object to ", name, 0);
		return 0;
		}

	return 1;
	}

// set parent object to given object address (run-time version)
// also affects sister pointers

void object::setparent (object *o) {
	object *p  = parent;
	lastparent = p;

	if (o != NULL)
		if (p->id == o->id) return;			// return if it's already there

	if (p) {											// was there a parent?
		p = p->child;                 		// what's the first child?
		if (p->id == id)							// is it me?
			parent->child = sister;		 		// then point ahead, possibly at NULL
		else {										// else find the sister and link that through this object
			while (p->sister->id != id) p = p->sister;
			p->sister = sister;
			}
		}

	flags.moved = 1;								// signal that object has moved
	parent      = o;								// set parent to the new object

	if (parent != NULL) {
		if (parent->child) {
			sister = parent->child->sister;		// resolve sister link
			parent->child->sister = this;
			}
		else {
			sister = NULL;       					// resolve parent/child link
			parent->child = this;
			}
		}
	}

// set the parent object of this to given object (load-time version)
// returns TRUE/FALSE depending on whether parent object was found

int object::setparent (char *s) {
	object *o = search (s);

	if (o) {
		setparent (o);
		lastparent = o;
		}
	else {
		error ("Could not set PARENT object to", name, 0);
		return 0;
		}

	return 1;
	}

// set the verbose description of an object

void object::setverbose (char *s) {
	if (!(verbose = new char [strlen (s) + 1])) catastrophy ("17");
	strcpy (verbose, s);
	}

// get the name of the object

char *object::getname (void) {
	return name;
	}

// get the short description of the object ("a-form")

char *object::getbrief (void) {
	return brief;
	}

// get the short description of the object ("the-form"), but if it's
// not there, use the "a-form"

char *object::getbrief2 (void) {
	return brief2;
	}

// get the long description of the object

char *object::getverbose (void) {
	return verbose;
	}

// get the parent of the object, if any

object *object::getparent (void) {
	return parent;
	}

// get the child of the object, if any

object *object::getchild (void) {
	return child;
	}

// get the sister object of the object, if any

object *object::getsister (void) {
	return sister;
	}

// find a certain object on key "name" (sequential version)

object *object::search (char *s) {
	object *o = object::root;
	while (o && strcmp (s, o->name)) o = o->next;
	return o;
	}

// find a certain object on a list of nouns (relational version),
// returns the "best fit". Takes a list of nouns for use in the search.

object *object::search (int *w, object *p) {
	object *o, *f = NULL;
	wordnode *v;
	int *u, i, j, max = 0;

	for (o = root->next; o; o = o->next) {
		j = (o->wordlist && *w) ? 1 : 0;
		for (i = 0, v = o->wordlist; v; v = v->next)
			for (u = w; *u; u++)
				if (v->number == *u) i++;
					else j = 0;									// j means "weak match"

		if (i) {													// if something there
			if (max) {											// if already a match
				if (i > max) f = o, max = i;				// matched even more?
				}
			else {												// if first match
				f = o, max = i;
				}
			if (j) if (p->rel (OR_RELATED, o)) f = o, max = i;
			}

		}

	return f;
	}

// find a certain object on a single word (associative version)
// returns the first match

object *object::search (int w, object *p) {
	wordnode *v;
	object *f = NULL;
	int i = 0;

	for (object *o = object::root; o; o = o->next)
		for (v = o->wordlist; v; v = v->next)
			if (v->number == w) {
				if (!i) {
					i = 1;
					f = o;
					}
				else {
					if (p->rel (OR_RELATED, o)) f = o;
					}
				}

	return f;
	}

// check relation between two objects
// note that the rule is: THIS in relation to P
//                        ----                -
// takes as parameters a relation and another object to relate to.
// thus, to find an object of a certain description, which must have
// the same parent (ie, a sister) as a reference object, use this function.

int object::rel (object_relation r, object *p) {
	int i = 0;

	switch (r) {
		case OR_IN:                                 // p must be parent
			if (parent->id == p->id) i = 1;
			break;
		case OR_NOTIN:                              // p musn't be parent
			if (parent->id != p->id) i = 1;
			break;
		case OR_WITH:                               // p must be sister
			if (parent->id == p->parent->id) i = 1;
			break;
		case OR_NOTWITH:                            // p musn't be sister
			if (parent->id != p->parent->id) i = 1;
			break;
		case OR_WITHIN:                             // p must be parent or sister
			if (parent->id == p->id ||
				 parent->id == p->parent->id) i = 1;
			break;
		case OR_NOTWITHIN:                          // p musn't be parent nor sister
			if (parent->id != p->id &&
				 parent->id != p->parent->id) i = 1;
			break;
		case OR_WITHINALL:                          // p must be parent, sister or child
			if (parent->id == p->id ||
				 parent->id == p->parent->id ||
				 id == p->parent->id) i = 1;
			break;
		case OR_WITHNOPARENT:                       // p must be in exit list
			if (isexit (p)) i = 1;
			break;
		case OR_RELATED:
			if (parent->id == p->id ||               // child?
				 parent->id == p->parent->id ||       // sister?
				 id == p->parent->id ||               // parent?
				 isexit (p) ||
				 isexit (p->parent) ||
				 p->isexit (this) ||
				 p->isexit (parent)) i = 1;           // exit?
			break;
		}

   return i;
	}

// is a certain object sister to this - done by comparing parents

int object::issister (object *o) {
	return (parent && o->parent->id == parent->id) ? 1 : 0;
	}

// is a certain object child to this

int object::ischild (object *o) {
	return o->parent && o->parent->id == id ? 1 : 0;
	}

// print out all sister objects

void object::listsisters (void) {
	object *r = parent;
	int i = 0;
	char *p;

	if (r) {
		r = r->child;
		while (r) {
			if (id != r->id) {
				if (i == 1) prettyprint (p);
				if (i > 1)  prettyprint (",", p);
				if (!i)     prettyprint (brief, messages.getmsg (10));
				i++;
				p = r->brief;
				}
			r = r->sister;
			}
		}

	if (i > 1) prettyprint (messages.getmsg (11));
	if (i)     prettyprint (p, ".");
	prettyprint ("\n");
	}

// list all child objects

void object::listchildren () {
	object *r = child;
	int i = 0;
	char *p;

	while (r) {
		if (id != r->id) {
			if (i == 1) {
				prettyprint (p);
				}
			if (i > 1) {
				prettyprint (",");
				prettyprint (p);
				}
			i++;
			p = r->brief;
			}
		r = r->sister;
		}

	if (i > 1) prettyprint (messages.getmsg (11));
	if (i) {
		prettyprint (p);
		prettyprint (".\n");
		}
	else prettyprint (messages.getmsg (14), "\n");
	}

// check for actors having moved relative to the current object and write
// out messages accordingly to their movements (has left / has entered)

void object::checkmovements (void) {
	for (object *o = root->next; o; o = o->next) {
		if (o->flags.actor &&                        // must be a person
			 o->flags.moved &&								// must have moved
			 o->parent->id == parent->id &&           // must have moved to our patch
			 o->id != id)                             // mustn't be p himself
				prettyprint (o->brief, messages.getmsg (37), parent->brief, ".\n");
		if (o->flags.actor &&								// must be a person
			 o->flags.moved &&								// must have moved
			 o->lastparent->id == parent->id &&  		// must have moved to our patch
			 o->id != id)										// mustn't be p himself
				prettyprint (o->brief, messages.getmsg (38), parent->brief, ".\n");
		}
	}

// list parent's verbose

void object::listparentverbose (void) {
	object *r = parent;
	if (r) prettyprint (r->verbose);
	}

// describe an object by parent's verbose and all sisters' brief information

void object::describe (void) {
	listparentverbose ();
	listsisters ();
	}

// load graphic information from disk

int object::grload (int n) {
	int i = 0;
	char* fnx = grfile;
	char* snx = "STUB.PIC";
	char tnx [25];
	FILE* fp;

	if (fnx == NULL) return 0;

	fp = fopen (fnx, "rb");
	if (fp == NULL) {
		prettyprint ("[INTERNAL ERROR:");
		prettyprint (fnx);
		prettyprint ("NOT FOUND, TRYING");
		prettyprint (snx);
		prettyprint ("]\n");
		fp = fopen (fnx = snx, "rb");
		if (fp == NULL) {
			prettyprint ("[INTERNAL ERROR:");
			prettyprint (fnx);
			prettyprint ("NOT FOUND EITHER. THIS IS SERIOUS]\n");
			fclose (fp);
			return 0;
			}
		}

	if (n) {
		if (picture.list != NULL) delete picture.list;
		picture.list = NULL;
		picture.size = NULL;
		do_anim = stub_anim;
		}

	i = 1;
	if (picture.list != NULL) delete picture.list;
	fread (&picture.size, sizeof (int), 1, fp);			// read size
	if (!(picture.list = new grcom [picture.size])) catastrophy ("18");				// allocate
	fread ((void*)picture.list, sizeof (grcom), picture.size, fp); // read pic
	fclose (fp);

	if (animflag) {
		if      (!strcmp (name, "TRANSIT1"))  do_anim = animate_airport;
		else if (!strcmp (name, "GATE"))      do_anim = animate_gate;
		else if (!strcmp (name, "CHECKIN"))   do_anim = animate_checkin;
		else if (!strcmp (name, "PLANE"))     do_anim = animate_plane;
		else if (!strcmp (name, "CAR"))       do_anim = animate_car;
		else if (!strcmp (name, "DEPT_HALL")) do_anim = animate_departure;
		else if (!strcmp (name, "AIRPORT_STREET")) do_anim = animate_airfield;
		else if (!strcmp (name, "TAXFREE"))   do_anim = animate_taxfree;
		else if (!strcmp (name, "FO_STREET")) do_anim = animate_fieldoff;
		else if (!strcmp (name, "SITE2"))     do_anim = animate_locusts;
		}

	return i;
	}

// draw current object's graphical information

void object::grdraw (void) {
	if (picture.list) draw (picture.list, picture.size);
	}

// clean up internal data structures

void object::cleanup (void) {
	for (object *o = root->next; o; o = o->next) {
		delete o->name;
		o->name = NULL;							// set to zero so destructor knows
		}
	}

// get all objects ready for a new pass

void object::ready (void) {
	for (object *o = root->next; o; o = o->next) {
		o->flags.moved = 0;
		o->lastparent  = o->parent;
		}
	}
