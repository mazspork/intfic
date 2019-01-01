// product.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// This file defines the products to be used as variables during the game.

// There are three types of standard products:
//
//  1) The normal user-manupulatable product,
//  2) The TIMER products, manipulated by the system,
//  3) The RANDOM products, returning a value between 0 and its value minus 1
//
// In addition, two very specific system products exist:
//
//  1) The TIMER product (without further characters), contains the
//     global time to be returned by the $TIME variable,
//  2) The GAME product, which is TRUE as long as the game is on.
//
// Products of type 2 and 3 must start with the letters TIMER and RANDOM
// respectively, and must have some extra letters to identify the actual
// product (no whitespace).

#include <string.h>
#include <stdlib.h>
#include "ifs.h"

// product symbol table constructor

product::product (void) {
	entries   = 0;				// total entries in product object
	timers    = 0;				// number of timer products
	randoms   = 0;				// number of random products
	namelist  = 0;				// list of names (deletes after initialization)
	timerlist = 0;				// list of indexes to timer values
	randlist  = 0;				// list of indexes to random values
	list      = 0;				// list of all product values, indexed by number

	global_timer = 0;			// the global time product index
	game_running = 0;			// the "game-is-still-on" product index
	}

// product symbol table destructor - deallocate all tables

product::~product (void) {
	if (namelist) {
		for (int i = 0; i < entries; i++) delete namelist [i];
		delete namelist;
		}
	if (list)      delete list;
	if (timerlist) delete timerlist;
	if (randlist)  delete randlist;
	}

// inititialize tables

void product::initialize (void) {
	if (entries) {
		if (!(list = new long  [entries])) catastrophy ("22");
		if (!(namelist = new char* [entries])) catastrophy ("25");
		if (timers) if (!(timerlist = new int [timers])) catastrophy ("23");
		if (randoms) if (!(randlist = new int [randoms])) catastrophy ("24");
		}
	entries   = 0;
	timers    = 0;
	randoms   = 0;
	}

// remove the character list when it's not needed anymore

void product::cleanup (void) {
	for (int i = 0; i < entries; i++) delete namelist [i];
	delete namelist;
	namelist = NULL;
	}

// enter a symbol into table

void product::entry (char *n, int v) {
	if (!(namelist [entries] = new char [strlen (n) + 1])) catastrophy ("26");
	strcpy (namelist [entries], n);

	list [entries] = v;

	if (!strncmp (n, "TIMER", 5))  {				// check for system product names
		timerlist [timers++]  = entries;
		if (!n [5]) global_timer = entries;
		}
	if (!strncmp (n, "RANDOM", 6)) randlist  [randoms++] = entries;
	if (!strcmp (n, "GAME")) game_running = entries;

	entries++;
	}

// add to the counters of normal products and timer products

void product::addcount (char *n) {
	entries++;											// one more entry
	if (!strncmp (n, "TIMER", 5))  timers++;	// one more timer entry?
	if (!strncmp (n, "RANDOM", 6)) randoms++;	// one more random entry?
	}

// find an entry's actual index value

int product::getindex (char *n) {
	if (!namelist) return NOTFOUND;

	for (int i = 0; i < entries; i++)
		if (!strcmp (namelist [i], n)) return i;

	return NOTFOUND;
	}

// match a product/value by relation

int product::match (int p, char r, int v) {
	int i = 0, value = list [p];        // assume normal entry

	for (int j = 0; j < randoms; j++)	// if it was a random entry,
		if (randlist [j] == p) {
			value = random (list [p]);    // take the random value
			break;
			}

	switch (r) {								// what's the relation?
		case '=':
			if (value == v) i = 1;			// equal
			break;
		case '>':
			if (value > v)  i = 1;			// greater than
			break;
		case '<':
			if (value < v)  i = 1;			// less than
			break;
		case '!':
			if (value != v) i = 1;			// not equal
			break;
		}

	return i;									// returns zero if no match
	}

// affect a product by value/relation

void product::affect (int p, char r, int v) {
	switch (r) {
		case '=':
			list [p] = v;
			break;
		case '+':
			list [p] += v;
			break;
		case '-':
			list [p] -= v;
			break;
		}
	}

// tick timers with a value

void product::elapse (long l) {
	for (int i = 0; i < timers; i++)			// update all timers
		if (list [timerlist [i]] > -1)		// but only if they're active
			list [timerlist [i]] += l;
	}
