#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"
#include "rate.inc"

/* rossler: a rossler oscillator */
/* Based on an example by Russell Borogove / www.tinygod.com */

static void rossler(float freq, float amp, float len_ms, int mono);

int main(int argc, char *argv[])
{
	float freq = 400.0f;
	float amp = 0.5f;
	float len_ms = 1000.0f;
	int mono = 0;
	int i;

	get_rate();

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-freq") && i+1 < argc)
			freq = atof(argv[++i]); /* frequency of the wave */
		else if (!strcmp(argv[i], "-amp") && i+1 < argc)
			amp = atof(argv[++i]);  /* amplitude from 0 to 1 */
		else if (!strcmp(argv[i], "-len") && i+1 < argc)
			len_ms = atof(argv[++i]); /* length in ms */
		else if (!strcmp(argv[i], "-mono"))
			mono = 1;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -freq arg, -amp arg, -len "
				"arg, -mono\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	rossler(freq, amp, len_ms, mono);
	return 0;
}

#define ROSSLER_SCALE     0.05757f
#define ROSSLER_ALT_SCALE 0.06028f

static void rossler(float freq, float amp, float len_ms, int mono)
{
	int len = (int)(len_ms * (RATE/1000.0f));
	int n;
	const float dt = 2.91f * freq / RATE;
	float dx, dy, dz, x, y, z, a, b, c;
	float f[2];

	dx = dy = dz = 0.0f;
	a = 0.15f;
	b = 0.2f;
	c = 10.0f;
	x = y = z = 1.0f;

	for (n = 0; n < len; n++)
	{
		/* Iterate the oscillator. */
		dx = -y - z;
		dy = x + a*y;
		dz = b + (x-c)*z;
		x += dx*dt;
		y += dy*dt;
		z += dz*dt;

		f[0] = amp * ROSSLER_SCALE*x;
		f[1] = amp * (mono ? ROSSLER_SCALE*x : ROSSLER_ALT_SCALE*y);
		if (fwrite(f, sizeof f[0], 2, stdout) < 2) break;
	}
}
