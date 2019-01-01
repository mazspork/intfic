// vocab.cpp

// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// Defines the vocabulary structure and symbol tables to be
// dynamically reconfigured and manipulated through the game.

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ifs.h"

// class vocabulary constructor

vocabulary::vocabulary (void) {
	dictionary = NULL;
	number     = 0;
	type       = NO_CLASS;
	wordcount  = 0;
	}

// class vocabulary destructor

vocabulary::~vocabulary (void) {
	for (int i = 0; i < wordcount; i++) delete dictionary [i].name;
	delete dictionary;
	}

// initialize the structures

void vocabulary::initialize (void) {
	if (!(dictionary = new namenode [wordcount])) catastrophy ("19");

	for (int i = 0; i < wordcount; i++) {
		dictionary [i].name   = NULL;
		dictionary [i].number = 0;
		dictionary [i].type   = NO_CLASS;
		dictionary [i].dual   = -1;
		}

   wordcount = 0;
	}

// increment word count

void vocabulary::addwordcount (void) {
	wordcount++;
	}

// add word to vocabulary (increases word counter)

void vocabulary::enter (char *s, wordclass t) {
	number++;
	type = t;
	addword (number, type, s);
	}

// add synonym to vocabulary (uses current word counter)

void vocabulary::synonym (char *s) {
	addword (number, type, s);
	}

// create a synonym for an existing word

void vocabulary::synonym (char *s, char *t) {
	for (int i = 0; i < wordcount; i++)
		if (!strcmp (dictionary [i].name, s))
			break;

	if (i == wordcount) {
		error ("No match for synonym", s, 0);
		return;
		}

	addword (dictionary [i].number, dictionary [i].type, t);
	}

// Actually add a word to the vocabulary, with the possibility of creating
// a link to a previously defined word of another word-class

void vocabulary::addword (int n, wordclass t, char *s) {
	int i = getnumber (s);
	if (i) {													// check for two words
		dictionary [i].dual         = wordcount;
		dictionary [wordcount].dual = i;
		}

	dictionary [wordcount].number = n;
	dictionary [wordcount].type   = t;
	if (!(dictionary [wordcount].name   = new char [strlen (s) + 1])) catastrophy ("20");
	strcpy (dictionary [wordcount].name, s);

	wordcount++;
	}

// return the number of a given word (look up the word)

int vocabulary::getnumber (char *s) {
	for (int i = 0; i < wordcount; i++)
		if (!strcmp (dictionary [i].name, s))
			return dictionary [i].number;

	return 0;
	}

// return the type of a given word (look up the type of the word)

wordclass vocabulary::gettype (char *s) {
	for (int i = 0; i < wordcount; i++)
		if (!strcmp (dictionary [i].name, s))
			return dictionary [i].type;

	return NO_CLASS;
	}

// return the type of a given word number (look up the type of the index)

wordclass vocabulary::gettype (int n) {
	for (int i = 0; i < wordcount; i++)
		if (dictionary [i].number == n)
			return dictionary [i].type;

	return NO_CLASS;
	}

// return the "other" object of a given number. this is used to overcome
// polyvalence!

int vocabulary::getother (int i) {
	if (dictionary [i].dual == -1)
		return -1;
	else
		return dictionary [dictionary [i].dual].number;
	}
