// graphics.cpp
//
// This file contains proprietary information of Maz Spork. Certain
// techniques contained within this file is confidential and must
// not be inspected without permission.
// Copyright (C) 1990 by Maz Spork.
//
// Handles graphics output in the game.

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <graphics.h>
#include <dos.h>
#include "ifs.h"

// externs

extern void (*do_anim) (void);
extern long gmt_time;
extern int upper;

// globals

int startline;							// where the text starts
int graphic_onscreen = 0;			// 1 if there's a picture on screen
int maxx, maxy;						// maximum values of onscreen coordinates
int maxcolour;							// maximum value of colour
int ypos;								// current y position on screen (for text)
int screenwidth;						// width of screen in characters
int chwidth, chheight;				// width and height of standard characters

double xaspect = 1, yaspect = 1;

int cga_table [] = {
	0, 1, 1, 3, 2, 2, 2, 3, 3, 1, 1, 3, 2, 2, 3, 3
	};

// externs

extern int gdriver;
extern int gmode;
extern int graphon;
extern int animflag;

// prototypes

void draw_time (int, int, float);
void panl (int, int, int, int, int);
void panr (int, int, int, int, int);

// initialize graphics

int ginit (void) {
	if (gdriver == -1) gdriver = DETECT;

	registerfarbgidriver (EGAVGA_driver_far);
	registerfarbgidriver (CGA_driver_far);
	registerfarbgidriver (IBM8514_driver_far);
	registerfarbgidriver (Herc_driver_far);

	registerfarbgifont (triplex_font_far);
	registerfarbgifont (sansserif_font_far);

	detectgraph (&gdriver, &gmode);

	switch (gdriver) {								// select appropriate graphics mode
		case VGA:										// guideline: always select a mode
			gdriver = VGA;								// with 640 pixels across screen.
			gmode = VGAMED;
			break;
		case EGA:
			gmode = EGAHI;
			break;
		case EGA64:
			gmode = EGA64HI;
			break;
		case CGA:
			gmode = CGAHI;
			break;
		case MCGA:
			gmode = MCGAHI;
			break;
		case HERCMONO:
			gmode = HERCMONOHI;
			break;
		case ATT400:
			gmode = ATT400HI;
			break;

      default:
			gdriver = CGA;
			gmode = CGAHI;
			break;
		}

	initgraph (&gdriver, &gmode, "");

	if (graphresult () != grOk) return 0;

	maxx        = getmaxx ();
	maxy        = getmaxy ();
	maxcolour   = getmaxcolor ();
	xaspect     = ((double) (maxx + 1)) / 640;
	yaspect     = ((double) (maxy + 1)) / 350;
	startline   = (int) (256.0 * yaspect);
	chwidth     = textwidth ("X");
	chheight    = textheight ("X");
	screenwidth = maxx / chwidth;
	}

// shut down graphics

void gclose (void) {
	if (graphon) closegraph ();
	}

// draw up the screen from command list

void draw (grcom *list, int size) {
	static int skewcolour = 0;

	graphic_onscreen = 0;

	setlinestyle (0, 0, 1);
	setfillstyle (SOLID_FILL, BLACK);
	bar (0, 0, maxx, startline - 1);
	setcolor (WHITE);
	line (0, startline - 1, maxx, startline - 1);

	int colour = maxcolour;
	int pattern = SOLID_FILL;
	int x, y, b;

	for (int i = 0; i < size; i++) {

		x = (int) (((double) list [i].x) * xaspect); // resolve aspect conflicts
		y = (int) (((double) list [i].y) * yaspect);

		switch (list [i].command) {
			case GR_INVALID:
				break;
			case GR_MOVE:
				moveto (x, y);
				break;
			case GR_PLOT:
				putpixel (x, y, colour);
				break;
			case GR_DRAW:
				lineto (x, y);
				break;
			case GR_COLOUR:
				colour = list [i].colour;

				if (colour > maxcolour) {				// bad colour for this mode
					skewcolour = 1;
					if (maxcolour == 1)
						colour = maxcolour;
					else
						colour = cga_table [colour];
					}
				else skewcolour = 0;

				setcolor (colour);
				break;
			case GR_PATTERN:
				pattern = list [i].pattern;
				break;
			case GR_FILL:
				b = list [i].border;

            if (b > maxcolour) b = maxcolour;

				if (skewcolour) {
					if (pattern == SOLID_FILL) {
						if (colour < maxcolour) {
							setfillstyle (pattern, maxcolour);
							floodfill (x, y, b);
							}
						}
					else {
						setfillstyle (pattern, colour);
						floodfill (x, y, b);
						}
					}
				else {
					setfillstyle (pattern, colour);
					floodfill (x, y, b);
					}
				break;
			case GR_THICKNESS:
				setlinestyle (0, 0, list [i].thickness);
				break;

			default:
				break;
			}
		}
	setcolor (WHITE);

	graphic_onscreen = 1;
	}

// clear graphics area

void clrgraph (void) {
	setfillstyle (SOLID_FILL, BLACK);
	bar (0, 0, maxx, startline - 1);
	}

// print a line of text

void printout (unsigned char *p) {
	if (p [strlen ((char*)p) - 1] == '\n') p [strlen ((char*)p) - 1] = '\0';
	outtextxy (0, ypos, (char*)p);
	ypos += chheight;
	moveto (0, ypos);

	if (ypos >= maxy - chheight * 2) {
		outtext ("(press return)");
		getch ();
		printreset ();
		}
	}

// print a character

void printout (unsigned char p) {
	if (p == 8) {
		setfillstyle (SOLID_FILL, BLACK);
		bar (getx () - chwidth, ypos, getx (), ypos + chwidth);
		moveto (getx () - chwidth, ypos);
		}
	else if (p == '\n' || p == NEWLINE) {
		ypos += chheight;
		moveto (0, ypos);
		}
	else if (p >= 32) {
		char q [2];
		q [0] = p;
		q [1] = 0;
		outtext (q);
		}
	}

// reset print system

void printreset (void) {
	setfillstyle (SOLID_FILL, BLACK);
	bar (0, startline, maxx, maxy);
	ypos = startline + 1;
	moveto (0, ypos);
	upper = 1;
	}

void normaltext (void) {
	setcolor (WHITE);
	}

void specialtext (void) {
	setcolor (CYAN);
	}

void outtexty (int y, char* s) {
	int x = maxx / 2 - textwidth (s) / 2;
	outtextxy (x, y, s);
	}

void titles (void) {
	setcolor (GREEN);

	setfillstyle (SOLID_FILL, GREEN);
	fillellipse (maxx/2,maxy/2,maxx/3 + 64,maxy/2);

	setcolor (WHITE);
	settextstyle (TRIPLEX_FONT, HORIZ_DIR, 2);

	outtexty (0, "ACTIVE LEARNING");

	setcolor (LIGHTMAGENTA);
	settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);

	outtexty (32, "a Computer-Aided Training & Evaluation System");

	setcolor (LIGHTCYAN);

	outtexty (80, "(C) Eva Arnvig, Nils Jensen & Maz Spork");

	setcolor (YELLOW);

	outtexty (110, "- credit - ");
	outtexty (150, "Program design: Maz Spork");
	outtexty (160, "   Game design: Eva Arnvig");
	outtexty (170, "                Nils Jensen");
	outtexty (180, "                Maz Spork");
	outtexty (190, "Graphic design: Maz Spork");
	outtexty (140, "       Concept: Eva Arnvig");

	setcolor (LIGHTRED);

	outtexty (260, "DEMONSTRATION GAME");
	}

// ANIMATIONS
// these functions are for animating the scenes

void animate (void) {
	if (graphic_onscreen) (*do_anim) ();
	}

// this is the stub routine if no graphics are on-screen

void stub_anim (void) {
	}

// animation in airport

void getdepartline (int l, void *b) {
	getimage (225, 84+l*4, 290, 85+l*4, b);
	}

void putdepartline (int l, void *b) {
	putimage (225, 84+l*4, b, COPY_PUT);
	}

// animation in departure hall

void animate_airport (void) {
	static entries = 6;
	static removing = 0;
	static counter = 0;

	unsigned size = imagesize (225, 84, 290, 86);
	void *bitmap = new char [size];

	if (!bitmap) return;

	if (!random (30) && entries < 6 && !removing) {				// add new one
		getdepartline (0, bitmap);
		putdepartline (entries, bitmap);
		entries++;
		}

	if (!random (30) && entries > 1 && !removing) {				// remove one
		removing = random (entries - 1);
		setfillstyle (SOLID_FILL, DARKGRAY);
		bar (225, 84+removing*4, 290, 84+removing*4);
		removing++;
		entries--;
		}

	if (++counter < 8) return;
	counter = 0;

	if (removing) {
		if (removing > entries) removing = 0;
		else {
			getdepartline (removing, bitmap);
			putdepartline (removing - 1, bitmap);
			setfillstyle (SOLID_FILL, DARKGRAY);
			bar (225, 84+removing*4, 290, 84+removing*4);
			removing++;
			}
		}

	delete bitmap;
	}

// animation by gate 24

void animate_gate (void) {
	unsigned size1 = imagesize (260, 41, 261, 59);
	unsigned size2 = imagesize (260, 41, 460, 59);

	void *bitmap1 = new char [size1];
	if (!bitmap1) return;
	void *bitmap2 = new char [size2];
	if (!bitmap2) return;

	getimage (260,41,260,59,bitmap1);

	getimage (261,41,418,59,bitmap2);
	putimage (260,41,bitmap2,COPY_PUT);

	getimage (425,41,425,59,bitmap2);
	putimage (418,41,bitmap2,COPY_PUT);

	getimage (426,41,583,59,bitmap2);
	putimage (425,41,bitmap2,COPY_PUT);

	getimage (590,41,590,59,bitmap2);
	putimage (583,41,bitmap2,COPY_PUT);

	getimage (591,41,639,59,bitmap2);
	putimage (590,41,bitmap2,COPY_PUT);

	putimage (639,41,bitmap1,COPY_PUT);

	delete bitmap1;
	delete bitmap2;
	}

void animate_checkin (void) {
	panl (0, 0, maxx, 29, 3);
	}

void animate_plane (void) {
	panl (0, 99, maxx, 119, 5);
	}

void animate_car (void) {
	panr (0, 112, maxx, 127, 1);
	panl (0, 193, maxx, 210, 3);
	}

void animate_airfield (void) {
	panl (0, 32, maxx, 36, 3);
	}

void animate_taxfree (void) {
	panr (160,35,414,51,3);
	}

void animate_locusts (void) {
	int x, y;
	putpixel (random (380), random (maxy), BLACK);
	for (int i = 0; i < 256; i++) {
		x = random (380);
		y = random (y);
		putpixel (x, y, y > 170 ? BROWN : LIGHTCYAN);
		}
	}

void animate_fieldoff (void) {
	panr (309, 21, maxx, 33, 1);
	}

void animate_departure (void) {
	for (int i = 0; i < 32; i++)
		putpixel (random (351 - 317) + 317, random (67 - 45) + 45,
			random (2) ? WHITE : BLACK);
	}

void panl (int x1, int y1, int x2, int y2, int speed) {
	unsigned size1 = imagesize (x1, y1, x2 - speed, y2);
	unsigned size2 = imagesize (x1, y1, speed, y2);

	void *bitmap1 = new char [size1];
	if (!bitmap1) return;
	void *bitmap2 = new char [size2];
	if (!bitmap2) return;

	getimage (x2 - speed, y1, x2, y2, bitmap2);

	getimage (x1, y1, x2 - speed, y2, bitmap1);
	putimage (speed + x1, y1, bitmap1, COPY_PUT);

	putimage (x1, y1, bitmap2, COPY_PUT);

	delete bitmap1;
	delete bitmap2;
	}

void panr (int x1, int y1, int x2, int y2, int speed) {
	unsigned size1 = imagesize (x1, y1, x2 - speed, y2);
	unsigned size2 = imagesize (x1, y1, x1 + speed, y2);

	void *bitmap1 = new char [size1];
	if (!bitmap1) return;
	void *bitmap2 = new char [size2];
	if (!bitmap2) return;

	getimage (x1, y1, x1 + speed, y2, bitmap2);
	getimage (x1 + speed, y1, x2, y2, bitmap1);
	putimage (x1, y1, bitmap1, COPY_PUT);
	putimage (x2 - speed, y1, bitmap2, COPY_PUT);

	delete bitmap1;
	delete bitmap2;
	}
