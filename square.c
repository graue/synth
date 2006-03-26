#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* square: generate a square wave */

#ifndef RATE
#define RATE 88200
#endif

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))

static float square(float input)
{
	input = fmod(input, 360.0f);
	if (input < 180.0f)
		return 1.0f;
	else
		return -1.0f;
}

int main(int argc, char *argv[])
{
	float freq = 1000, amp = 0.5f, phase = 0.0f;
	float period;
	long len = RATE;
	long n;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-freq") && ++i < argc)
			freq = atof(argv[i]); /* frequency of the wave */
		else if (!strcmp(argv[i], "-amp") && ++i < argc)
			amp = atof(argv[i]);  /* amplitude from 0 to 1 */
		else if (!strcmp(argv[i], "-len") && ++i < argc)
			len = atof(argv[i]) / 1000.0f * RATE; /* length in ms */
		else if (!strcmp(argv[i], "-phase") && ++i < argc)
			phase = atof(argv[i]); /* phase in degrees */
	}

	/* check options */
	freq  = CLAMP(0.01f, freq, RATE / 22.0f * 10.f);
	amp   = CLAMP(0.0f, amp, 1.0f);
	phase = CLAMP(0.0f, phase, 359.99f);

	/* convert options */
	amp *= 32768.0f;
	period = RATE / freq;

	for (n = 0; n < len; n++)
	{
		float f;
		char *fr = (char *)&f;

		f = square((n + phase) * 360.0f / period) * amp;

		/* left channel */
		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fr[i]);

		/* right channel */
		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fr[i]);
	}

	return 0;
}
