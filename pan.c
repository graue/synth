#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* pan.c: adjust a waveform's horizontal panning */

static void pan(float angle);

int main(int argc, char *argv[])
{
	float angle = 0.0f;
	int i;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-angle") && i+1 < argc)
			angle = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -angle arg (-180 to 180)\n");
			exit(0);
		}
	}

	/* check options */
	angle = CLAMP(-180.0f, angle, 180.0f);

	/* convert options */
	angle = angle * M_PI / 180.0f;

	SET_BINARY_MODE
	pan(angle);
	return 0;
}

static void pan(float angle)
{
	float f;
	float leftamp, rightamp;
	int odd = 0;

	leftamp  = cos(angle) + sin(angle);
	rightamp = cos(angle) - sin(angle);

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		odd = !odd;

		if (odd) /* left speaker */
			f *= leftamp;
		else     /* right speaker */
			f *= rightamp;

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
