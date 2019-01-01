
// draw up the time digitally

void draw_digit (int digit, int x_coord, int y_coord, float magni) {
	static int digital [10][18] = {
		{ 4, 0,0, 1,0, 1,2, 0,2, 0,0                },
		{ 1, 1,0, 1,2                               },
		{ 5, 0,0, 1,0, 1,1, 0,1, 0,2, 1,2           },
		{ 6, 0,0, 1,0, 1,1, 0,1, 1,1, 1,2, 0,2      },
		{ 4, 0,0, 0,1, 1,1, 1,0, 1,2                },
		{ 5, 1,0, 0,0, 0,1, 1,1, 1,2, 0,2           },
		{ 4, 0,0, 0,2, 1,2, 1,1, 0,1                },
		{ 2, 0,0, 1,0, 1,2                          },
		{ 7, 0,1, 0,2, 1,2, 1,1, 0,1, 0,0, 1,0, 1,1 },
		{ 4, 1,2, 1,0, 0,0, 0,1, 1,1                }
		};

	int count = digital [digit] [0] * 2 + 3;
	moveto (x_coord + digital [digit] [1] * magni,
			  y_coord + digital [digit] [2] * magni);

	for (int i = 3; i < count; i+=2)
		lineto (x_coord + digital [digit] [i] * magni,
				  y_coord + digital [digit] [i+1] * magni);

	}


void draw_time (int x_coord, int y_coord, float magni) {
	static int seconds = -1;

	struct date dt;
	struct time tm;
	int spc = (magni * 1.2);

	unixtodos (gmt_time, &dt, &tm);

	if (seconds == tm.ti_sec) return;

	int ox = getx ();
	int oy = gety ();

	setfillstyle (SOLID_FILL, DARKGRAY);
	bar (x_coord, y_coord, x_coord+magni, y_coord+magni);

	seconds = tm.ti_sec;

	draw_digit (tm.ti_hour / 10, x_coord, y_coord, magni);
	x_coord += spc;
	draw_digit (tm.ti_hour % 10, x_coord, y_coord, magni);
	x_coord += spc;
	draw_digit (tm.ti_min / 10, x_coord, y_coord, magni);
	x_coord += spc;
	draw_digit (tm.ti_min % 10, x_coord, y_coord, magni);
	x_coord += spc;
	draw_digit (tm.ti_sec / 10, x_coord, y_coord, magni);
	x_coord += spc;
	draw_digit (tm.ti_sec % 10, x_coord, y_coord, magni);

	moveto (ox, oy);
	}

