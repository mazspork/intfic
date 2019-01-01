// message.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// System message symbol table.

// class messages is necessary because:-
//
//   1) system messages must be in different national languages,
//   2) some messages occur very frequently in events and can be
//      unified through this class

#include <string.h>
#include <stdlib.h>
#include "ifs.h"

// message table constructor: reset entries and string table

message::message (void) {
	entries  = 0;
	namelist = 0;
	}

// message table destructor: deallocate all allocated entries and the table

message::~message (void) {
	for (int i = 0; i < entries; i++) delete namelist [i];
	delete namelist;
	}

// tell the message class what's the current highest message number

void message::addcount (int i) {
	if (i > entries) entries = i;
	}

// initialize the message system after all messages have been scanned

void message::initialize (void) {
	namelist = new char* [entries];
	for (int i = 0; i <= entries; i++) namelist [i] = NULL;
	}

// enter a string into a message slot (won't if one's already there)

void message::entry (int i, char *s) {
	if (!namelist [i]) {
		namelist [i] = new char [strlen (s) + 1];
		strcpy (namelist [i], s);
		}
	}

// get a message from the message symbol table

char *message::getmsg (int i) {
	return namelist [i];
	}

