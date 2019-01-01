#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <dos.h>

const antal = 384;
const border = 8;

void main (void){
	int gdriver = DETECT, gmode;
	initgraph(&gdriver, &gmode, "");
	int errorcode = graphresult();

	if (errorcode != grOk) {
		printf("Graphics error: %s\n", grapherrormsg(errorcode));
		printf("Press any key to halt:");
		getch();
		exit(1);             /* return with error code */
		}

	randomize ();

	int v [antal] [4];
	int c [antal] [2];
	int m [2];

	m [0] = getmaxx ();
	m [1] = getmaxy ();

	for (int j = 0; j < 4; j++) {
		c [j] [0] = random (m [j & 1]);
		c [j] [1] = random (border);
		}

	int i = 1;
	int maxcolour = getmaxcolor ();

	int p = 0;
	int temp = 0;

	while(!kbhit()) {

		delay (25);

		if (random (100) == 50) while (!(i = random (maxcolour)));
		setcolor(i);
		line(c[0][0],c[1][0],c[2][0],c[3][0]);
		for(j=0;j<4;j++) v[p][j]=c[j][0];
		if(++p==antal) { p=0; temp=1; }
		setcolor (0);
		if(temp) line(v[p][0],v[p][1],v[p][2],v[p][3]);
		for(j=0;j<4;j++)
			if (random(1000)==500)
				while (!(c[j][1]=random(border)));
		for (j=0;j<4;j++) {
			if (((c[j][0]+c[j][1])>=m[j&1])||((c[j][0]+c[j][1])<=0))
				c[j][1]=-c[j][1];
			c[j][0]+=c[j][1];
			}
		}
	getch();
	closegraph();
	}
