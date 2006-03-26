#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* pan.c: adjust a waveform's horizontal panning */

#ifndef PI
#define PI 3.1415927f
#endif

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void pan(float angle);

int main(int argc, char *argv[])
{
	float angle = 0.0f;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-angle") && ++i < argc)
			angle = atof(argv[i]);
	}

	/* check options */
	angle = CLAMP(-180.0f, angle, 180.0f);

	/* convert options */
	angle = angle * PI / 180.0f;

	pan(angle);
	return 0;
}

#define SQRT2 1.414214f

static void pan(float angle)
{
	float f;
	float leftamp, rightamp;
	char *fw;
	int c, i, odd = 0;

	leftamp  = cos(angle) + sin(angle);
	rightamp = cos(angle) - sin(angle);

	fw = (char *)&f;

	for (;;)
	{
		odd = !odd;

		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = c;
		}

		if (odd) /* left speaker */
			f *= leftamp;
		else     /* right speaker */
			f *= rightamp;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}
