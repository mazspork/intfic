// scroll the screen

void scroll () {

	line (0, maxy, maxx, 0);

	unsigned s = imagesize (0, 0, maxx, 8);	// bytecount size of a line
	unsigned d = 65535 / s;							// lines we can move in 1 go
	unsigned i = startline;							// starting point
	unsigned e = (maxy - 8 - startline) / 8;	// lines to actually move

	if (d > e) d = e;

	char *buffer = new char [s * d]; 	// allocate mem for max lines

	while (i <= maxy - d * 8) {			// copy as much as we can
		getimage (0, i + 8, maxx, d * 8, buffer);
		putimage (0, i, buffer, COPY_PUT);
		i += d * 8;
		}

	if (i <= maxy - d * 8) {            // copy the rest if there's more
		getimage (0, i + 8, maxx, maxy, buffer);
		putimage (0, i, buffer, COPY_PUT);
		}

	setfillstyle (SOLID_FILL, BLACK);	// blank out the bottom line
	bar (0, maxy - 8, maxx, maxy);
	delete buffer;
	}

