// screens.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// This file handles the application-specific screens with input
// fields etc.

#include <graphics.h>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include "ifs.h"

// externs

extern double xaspect, yaspect;
extern int maxx, maxy;
extern int ypos;
extern player_data pdata;

// globals

void letterhead (void);					// draws up the "international development" letter head
void fix (char *);

char *text [3] = {
	"INTERNATIONAL    DEVELOPMENT",
	"Application Form",
	"Letter of Assignment"
	};

// function bodies. This function MOVES the cursor, but also sets the
// global y coordinate posistion

void mymoveto (int x, int y) {
	moveto (x, y);
	ypos = y;
	}

// draw "International Development" header - EGA/VGA version

void draw_header (void) {
	do {

		letterhead ();
		settextstyle (SANS_SERIF_FONT, HORIZ_DIR, 2);
		moveto (120, 40);
		outtextxy (250, 44, text [1]);

		setcolor (GREEN);

		line (40, 0, 40, 80);
		line (maxx - 40, 0, maxx - 40, 80);
		line (0, 80, maxx, 80);
		line (0, 82, maxx, 82);
		line (0, 104, maxx, 104);
		line (320, 104, 320, 126);
		line (0, 126, maxx, 126);
		line (80, 126, 80, 148);
		line (132, 126, 132, 148);
		line (0, 148, maxx, 148);

		rectangle (0, 0, maxx, 150);

		setcolor (YELLOW);

		settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);

		outtextxy (  2,  84, "Full name (given and family)");
		outtextxy (  2, 106, "Nationality");
		outtextxy (322, 106, "Home country");
		outtextxy (  2, 128, "Sex");
		outtextxy ( 82, 128, "Age");
		outtextxy (134, 128, "Mother tongue");

		setcolor (WHITE);

		settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);

		char n [71];

		mymoveto (10, 94);
		input (n, 70);
		mymoveto (10, 116);
		input (pdata.nationality, 30);
		mymoveto (330, 116);
		input (pdata.country, 30);

		do {
			mymoveto (10, 138);
			input (pdata.sex, 7);
			char c = mytolower (*pdata.sex);

			if (c == 'f'|| c == 'k') {
				strcpy (pdata.lastname, "Ms. ");
				strcpy (pdata.sex, "female");
				}
			else if (c == 'm' || c == 'h') {
				strcpy (pdata.lastname, "Mr. ");
				strcpy (pdata.sex, "male");
				}
			else *pdata.sex = '?';
			} while (*pdata.sex == '?');

		mymoveto (90, 138);
		input (pdata.age, 2);
		mymoveto (142, 138);
		input (pdata.tongue, 40);

		char *p1, *p2 = n, *p3;
		p1 = getnext (p2);
		strcpy (pdata.firstname, p1);
		while (p2) p1 = getnext (p2);
		strcpy (&pdata.lastname [4], p1);
		fix (&pdata.lastname [4]);
		fix (pdata.firstname);
		fix (pdata.country);
		fix (pdata.tongue);
		fix (pdata.nationality);

		outtextxy (240, 340, "Is everything ok?");
		while (!kbhit ());
		} while (mytolower (getch ()) == 'n');

	}

void draw_assignment (void) {
	letterhead ();

	settextstyle (SANS_SERIF_FONT, HORIZ_DIR, 2);
	moveto (120, 40);
	outtextxy (220, 44, text [2]);

	setcolor (GREEN);

	line (40, 0, 40, 80);
	line (maxx - 40, 0, maxx - 40, 80);
	line (0, 80, maxx, 80);
	line (0, 82, maxx, 82);
	rectangle (0, 0, maxx, 330);

	setcolor (WHITE);
	settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);

	outtextxy (64, 90, pdata.lastname);
	outtextxy (64, 100, pdata.country);

	outtextxy ( 64, 115, "Project no. INTDEV/UIU/82/001");
	outtextxy ( 64, 130, "Sector: Natural Resources");
	outtextxy ( 80, 150, "You are herby appointed as project leader of the");
	outtextxy ( 80, 160, "reforestation/dune fixation/water restauration");
	outtextxy ( 80, 170, "project in Faraway.");
	outtextxy ( 80, 190, "Your terms of employment imply that you within");
	outtextxy ( 80, 200, "the time frame of three years succeed in carrying");
	outtextxy ( 80, 210, "out reforestation of 12.000 ha. of land in the");
	outtextxy ( 80, 220, "Mongrovia area.");
	outtextxy ( 80, 240, "Your budget is one million dollars.");
	outtextxy ( 80, 260, "Please report as soon as possible to the national");
	outtextxy ( 80, 270, "ID office in Faraway.");
	outtextxy (140, 290, "Good luck.");

	outtextxy (220, 340, "(press a key to continue)");
	getch ();
	}

// draw up the letter head

void letterhead (void) {
	cleardevice ();

	setcolor (LIGHTCYAN);
	setfillstyle (SOLID_FILL, LIGHTCYAN);
	fillellipse (maxx / 2, 20, 20, 16);

	settextstyle (TRIPLEX_FONT, HORIZ_DIR, 4);
	setcolor (WHITE);
	outtextxy (55, 2, text [0]);
	}

// fix uppercase/lowercase

void fix (char *p) {
	if (*p) {
		*p = mytoupper (*p);
		p++;
		while (*p) {
			*p = mytolower (*p);
			p++;
			}
		}
	}
