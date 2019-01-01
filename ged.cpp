// ged.cpp
//
// Graphics EDitor
//
// this file contains parts of the IFS graphics subsystem. It defines the
// graphics primitives used for presenting graphical information.
//
// (C) 1990, 1991 Maz Spork

// ! Use DrawPoly if there are lots of points in one figure


#include <graphics.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// prototypes

int gdriver = DETECT, gmode;
char buf [2];
int x = 0, y = 0;
int ox, oy, oc;
int lastx = x, lasty = y;
int colour = WHITE;
int i, c;
int pattern = SOLID_FILL;
int thickness = 1;
int maxcolour, border;
int maxx, maxy;
int orgmode;
int skewcolour = 0;
double xaspect, yaspect;

int cga_table [] = {
	0, 1, 1, 3, 2, 2, 2, 3, 3, 1, 1, 3, 2, 2, 3, 3
	};

void draw_exterior (void);
void pr (int, char *);

// Normal keyboard codes

const
			Tab =   9,
			Esc =  27,
		 Break =   3,

// Extended keyboard codes (buffer holds '\0' + code)


	Backspace =   8,
			 Up =  72,
		  Down =  80,
		  Left =  75,
	 CtrlLeft = 115,
		 Right =  77,
	CtrlRight = 116,
			Ins =  82,
			Del =  83,
		  Home =  71,
			End =  79,
		  PgUp =  73,
	 CtrlPgUp = 132,
		  PgDn =  81,
	 CtrlPgDn = 118,
			 F1 =  59,
			 F2 =  60,
			 F3 =  61,
			 F4 =  62,
			 F5 =  63,
			 F6 =  64,
			 F7 =  65,
			 F8 =  66,
			 F9 =  67,
			F10 =  68;

// Graphics primitives

enum grcomnum {
	GR_INVALID,			// invalid graphic command
	GR_MOVE,				// move pointer to (x,y)
	GR_PLOT,				// plot a point at (x,y)
	GR_DRAW,				// draw a line to (x,y)
	GR_FILL,				// flood fill at (x,y)
	GR_COLOUR,			// change colour to (colour);
	GR_PATTERN,			// change pattern to (pattern);
	GR_THICKNESS,		// toggle thickness value
	};

struct grcom {
	grcomnum command;	// graphics command to execute
	int x, y;			// the coordinates of the object
	union {
		int radius;		// the radius of a circle, or
		int colour;		// the colour of an object, or
		int pattern;	// the pattern of an object
		int border;		// the border of a flood fill
		int thickness;	// thickness of line
		};
	};

class graphic {
	grcom *list;
	int size;
	int ptr;

public:
	graphic ();
	~graphic (void);

	void insert (grcomnum,int,int,int);
	void insert (grcomnum,int,int);
	void insert (grcomnum,int);
	void draw ();
	void save (FILE *);
	void load (FILE *);
	void printcommand (void);
	void printcommand (int);
	void remove (void);
	void setvals (void);
	int getsize (void);
	void yankup (void);
	void yankdown (void);
	void addcursor (void) { if (ptr < size - 1) ptr++; }
	void subcursor (void) { if (ptr > 0) ptr--; }
	void tostart (void) { ptr = 0; }
	void toend (void) { ptr = size; }
	};

int graphic::getsize (void) {
	return size;
	}

void graphic::remove (void) {
	if (size) yankdown ();
	}

void graphic::yankup (void) {
	for (int i = size; i > ptr; i--) list [i] = list [i - 1];
	size++;
	}

void graphic::yankdown (void) {
	if (size && ptr) {
		for (int i = ptr; i < size; i++) list [i - 1] = list [i];
		size--;
		ptr--;
		}
	}

// class graphic insertion of object

void graphic::insert (grcomnum c, int x, int y, int a) {
	yankup ();

	list [ptr].command = c;
	list [ptr].x = x;
	list [ptr].y = y;
	list [ptr].radius = a;		// radius, colour or pattern - all the same
	ptr++;
	}

void graphic::insert (grcomnum c, int x, int y) {
	yankup ();

	list [ptr].command = c;
	list [ptr].x = x;
	list [ptr].y = y;
	ptr++;
	}

void graphic::insert (grcomnum c, int a) {
	yankup ();

	list [ptr].command = c;
	list [ptr].radius = a;
	ptr++;
	}

// class graphic constructor

graphic::graphic () {
	size = ptr = 0;
	list = new grcom [2048];
	}

// class graphic destructor

graphic::~graphic (void) {
	delete list;
	}

// draw up the shape on the screen

void graphic::draw (void) {
	int colour = WHITE;
	int pattern = SOLID_FILL;
	setlinestyle (0, 0, 1);

	int x, y, b;

	for (int i = 0; i < size; i++) {

		x = (int) (((double) list [i].x) * xaspect);
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
				if (colour > maxcolour) {
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
	}

void graphic::printcommand (void) {
	setfillstyle (SOLID_FILL,BLACK);
	bar (400,272,maxx,279);

	if (ptr)
		printcommand (ptr - 1);
	else
		printcommand (0);
	}

void graphic::printcommand (int i) {
	char y [0x20];

	sprintf (y, "%4d:", i);
	moveto (400,272);
	setcolor (WHITE);
	outtext (y);

	if (!size)
		outtext ("*EMPTY*");

	else switch (list [i].command) {
		case GR_INVALID:
			outtext ("*INVALID*");
			break;
		case GR_MOVE:
			outtext ("MOVETO ");
			pr (list [i].x, ",");
			pr (list [i].y, "");
			break;
		case GR_PLOT:
			outtext ("PUTPIXEL ");
			pr (list [i].x, ",");
			pr (list [i].y, "");
			break;
		case GR_DRAW:
			outtext ("LINETO ");
			pr (list [i].x, ",");
			pr (list [i].y, "");
			break;
		case GR_COLOUR:
			outtext ("SETCOLOR ");
			*y = list [i].colour + 'A';
			*(y+1) = 0;
			outtext (y);
			break;
		case GR_PATTERN:
			outtext ("PATTERN ");
			*y = list [i].pattern + 'A';
			*(y+1) = 0;
			outtext (y);
			break;
		case GR_FILL:
			outtext ("FLOODFILL ");
			pr (list [i].x, ",");
			pr (list [i].y, ",");
			*y = list [i].border + 'A';
			*(y+1) = 0;
			outtext (y);
			break;
		case GR_THICKNESS:
			outtext ("THICKNESS toggle");
			break;

		default:
			outtext ("*INVALID*");
			break;
		}

	}

void graphic::save (FILE *fp) {
	fwrite (&size, sizeof (int), 1, fp);
	fwrite (list, sizeof (grcom), size, fp);
	}

void graphic::load (FILE *fp) {
	fread (&size, sizeof (int), 1, fp);
	fread (list, sizeof (grcom), size, fp);
	setvals ();
	ptr = size;
	}

void graphic::setvals (void) {
	x = 0, y = 0;
	colour = WHITE;
	pattern = SOLID_FILL;

	for (int i = 0; i < ptr; i++) {
		switch (list [i].command) {
			case GR_COLOUR:
				colour = list [i].colour;
				break;
			case GR_PATTERN:
				pattern = list [i].pattern;
				break;
			default:
				x = list [i].x;
				y = list [i].y;
				lastx = x, lasty = y;
				break;
			}
		}
	}

void setaspects () {
	maxx = getmaxx ();
	maxy = getmaxy ();
	maxcolour = getmaxcolor ();
	xaspect = (((double)(maxx+1)/640));
	yaspect = (((double)(maxy+1)/350));
	}

void pr (int x, char *z) {
	char y [0x20];
	sprintf (y, "%d", x);
	outtext (y);
	outtext (z);
	}

void main (void) {
	int run = 1;
	char filename [0x20];
	FILE *fp;
	int i, j;

	graphic *picture = new graphic;

	initgraph (&gdriver, &gmode, "");

	if (graphresult () != grOk) {
		puts ("No graphics!!! ... ?");
		exit (10);
		}

	orgmode = getgraphmode ();

	setaspects ();

	border = maxcolour;
	buf [1] = '\0';
	grcom testcoms [512];

	draw_exterior ();
	picture->draw ();

	setfillstyle (SOLID_FILL, WHITE);

	while (run) {
		ox = x, oy = y;			// save x and y

		oc = getpixel (x, y);

		setcolor (WHITE);

		putpixel (x, y, WHITE);

		c = getch ();

		if (c == Break) run = 0;

		if (!c) {					// extd. code, eg. function key or arrow
			switch (getch ()) {  // get extended code
				case Right:
					x++;
					break;
				case Left:
					x--;
					break;
				case Up:
					y--;
					break;
				case Down:
					y++;
					break;
				case PgUp:
					y-= 16;
					break;
				case PgDn:
					y+= 16;
					break;
				case CtrlLeft:
					x-= 16;
					break;
				case CtrlRight:
					x+= 16;
					break;
				case Home:
					picture->tostart ();
					picture->printcommand ();
					break;
				case End:
					picture->toend ();
					picture->printcommand ();
					break;
				case F9:
					closegraph ();
					printf ("Which driver? ");
					gdriver = getch () - '0';
					printf ("\nWhich mode? ");
					gmode = getch () - '0';
					initgraph (&gdriver, &gmode, "");
					setaspects ();
					ungetch ('t');
					break;
				case F10:
					closegraph ();
					gdriver = DETECT;
					gmode = 0;
					initgraph (&gdriver, &gmode, "");
					setaspects ();
					ungetch ('t');
					break;

				default:			// erroneous command
					break;
				}

			if (x > maxx || x < 0) x = ox;
			if (y > 255 || y < 0) y = oy;

			}
		else {

			switch (toupper (c)) {

				case '-':
				case '_':
					picture->subcursor ();
					picture->printcommand ();
					break;

				case '+':
				case '=':
					picture->addcursor ();
					picture->printcommand ();
					break;

				case Backspace:
					setcolor (WHITE);
					*buf = 'A' + colour;
					outtextxy (colour * 16 + 4, 296, buf);
					setcolor (WHITE);
					*buf = 'A' + pattern;
					outtextxy (pattern * 16 + 4, 326, buf);
					setcolor (BLACK);
					line (border * 16 + 4, 304, border * 16 + 12, 304);
					picture->remove ();
					picture->setvals ();
					picture->printcommand ();
					setfillstyle (SOLID_FILL, BLACK);
					bar (0, 0, maxx, (256 * yaspect) - 1);
					picture->draw ();
					picture->setvals ();
					x = ox, y = oy;
					oc = getpixel (ox, oy);
					setcolor (RED);
					*buf = 'A' + colour;
					outtextxy (colour * 16 + 4, 296, buf);
					setcolor (RED);
					*buf = 'A' + pattern;
					outtextxy (pattern * 16 + 4, 326, buf);
					setcolor (WHITE);
					line (border * 16 + 4, 304, border * 16 + 12, 304);
					break;
				case 'M':
					moveto (x, y);
					lastx = x, lasty = y;
					picture->insert (GR_MOVE, x, y);
					picture->printcommand ();
					break;

				case 'P':
					oc = colour;
					putpixel (x, y, colour);
					lastx = x, lasty = y;
					picture->insert (GR_PLOT, x, y);
					picture->printcommand ();
					break;

				case 'D':
					oc = colour;
					moveto (lastx, lasty);
					setcolor (colour);
					setlinestyle (0,0,thickness);
					lineto (x, y);
					setlinestyle (0,0,1);
					lastx = x, lasty = y;
					picture->insert (GR_DRAW, x, y);
					picture->printcommand ();
					break;

				case 'C':
					setcolor (WHITE);
					*buf = 'A' + colour;
					outtextxy (colour * 16 + 4, 296, buf);
					outtextxy (300,272,"Colour?");
					colour = toupper (getch ()) - 'A';
					setfillstyle (SOLID_FILL, BLACK);
					bar (300,272,364,279);
					setcolor (RED);
					*buf = 'A' + colour;
					outtextxy (colour * 16 + 4, 296, buf);
					picture->insert (GR_COLOUR, colour);
					picture->printcommand ();
					break;

				case 'F':
					putpixel (ox, oy, oc);
					setcolor (colour);
					setfillstyle (pattern, colour);
					floodfill (x, y, border);
					picture->insert (GR_FILL, x, y, border);
					picture->printcommand ();
					oc = getpixel (ox, oy);
					break;

				case 'A':
					setcolor (WHITE);
					*buf = 'A' + pattern;
					outtextxy (pattern * 16 + 4, 326, buf);
					outtextxy (300,272,"Pattern?");
					pattern = toupper (getch ()) - 'A';
					setfillstyle (SOLID_FILL, BLACK);
					bar (300,272,364,279);
					setcolor (RED);
					*buf = 'A' + pattern;
					outtextxy (pattern * 16 + 4, 326, buf);
					picture->insert (GR_PATTERN, pattern);
					picture->printcommand ();
					break;

				case 'B':
					setcolor (BLACK);
					line (border * 16 + 4, 304, border * 16 + 12, 304);
					setcolor (WHITE);
					outtextxy (300,272,"Border?");
					border = toupper (getch ()) - 'A';
					setfillstyle (SOLID_FILL, BLACK);
					bar (300,272,364,279);
					setcolor (WHITE);
					line (border * 16 + 4, 304, border * 16 + 12, 304);
					picture->printcommand ();
					break;

				case 'T':
					picture->setvals ();
					draw_exterior ();

				case 'R':
					setfillstyle (SOLID_FILL, BLACK);
					bar (0,0,maxx,(256*yaspect)-1);
					picture->draw ();
					picture->setvals ();
					break;

				case 'S':
					restorecrtmode ();
					printf ("Saving file\nFilename? ");
					gets (filename);
					fp = fopen (filename, "wb");
					picture->save (fp);
					fclose (fp);
					printf ("%s saved\n", filename);
					setgraphmode (getgraphmode ());
					draw_exterior ();
					picture->draw ();
					oc = getpixel (x, y);
					picture->printcommand ();
					break;

				case 'L':
					restorecrtmode ();
					printf ("Loading file\nFilename? ");
					gets (filename);
					if ((fp = fopen (filename, "rb")) != NULL) {
						picture->load (fp);
						fclose (fp);
						printf ("%s loaded\n", filename);
						}
					setgraphmode (getgraphmode ());
					picture->setvals ();
					draw_exterior ();
					picture->draw ();
					ox = 0;
					oy = 0;
					oc = getpixel (ox, oy);
					picture->printcommand ();
					break;

				case 'U':
					thickness = 4 - thickness;
					picture->insert (GR_THICKNESS, thickness);
					picture->printcommand ();
					break;

				default:
					break;
				}
			}

		setfillstyle (SOLID_FILL,BLACK);
		bar (0,272,256,279);
		setcolor (WHITE);
		moveto (8,272);
		outtext ("(");
		pr (x, ",");
		pr (y, ")");

		moveto (128,272);
		outtext ("Size ");
		pr (picture->getsize () * sizeof (grcom), " bytes");

		putpixel (ox, oy, oc);
		}

	closegraph ();

	delete picture;
	}

// draw all

void draw_exterior (void) {
	for (i = 0; i <= maxcolour; i++) {
		setfillstyle (SOLID_FILL, i);
		bar (i * 16, 280, i * 16 + 14, 294);
		setcolor (WHITE);
		*buf = 'A' + i;
		outtextxy (i * 16 + 4, 296, buf);
		}

	setcolor (WHITE);

	for (i = EMPTY_FILL; i < USER_FILL; i++) {
		setfillstyle (i, maxcolour);
		bar (i * 16, 310, i * 16 + 14, 324);
		*buf = 'A' + i;
		outtextxy (i * 16 + 4, 326, buf);
		}

	setcolor (WHITE);
	outtextxy (300, 288, "Use: Cursor keys to move around,");
	outtextxy (300, 296, "D to draw lines, C to colour, A to");
	outtextxy (300, 304, "pattern, F to flood fill, P to plot,");
	outtextxy (300, 312, "M to move point, B to set new border,");
	outtextxy (300, 320, "R to re-draw, S to save, L to load,");
	outtextxy (300, 328, "U to set line thickness, <tab> to");
	outtextxy (300, 336, "move between objects (+/-/End/Home).");

	setcolor (maxcolour);
	line (0, 256 * yaspect, maxx, 256 * yaspect);

	setcolor (YELLOW);
	outtextxy (185, 260, "Maz' Quick'n' Dirty Paint Program");
	setcolor (RED);
	*buf = 'A' + colour;
	outtextxy (colour * 16 + 4, 296, buf);
	*buf = 'A' + pattern;
	outtextxy (pattern * 16 + 4, 326, buf);
	setcolor (WHITE);
	line (border * 16 + 4, 304, border * 16 + 12, 304);
	}

