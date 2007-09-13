#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"
#include "rate.inc"

/* octagon: generate an octagon wave */

/*
   _
 _/ \_
      \_/
*/

static float octagon(float input)
{
	input = fmod(input, 360.0f);
	input /= 45.0f;
	if (input < 1.0f) /* - */
		return 0.0f;
	else if (input < 2.0f) /* / */
		return input - 1.0f;
	else if (input < 3.0f) /* - */
		return 1.0f;
	else if (input < 4.0f) /* \ */
		return 1.0f - (input - 3.0f);
	else if (input < 5.0f) /* - */
		return 0.0f;
	else if (input < 6.0f) /* \ */
		return -(input - 5.0f);
	else if (input < 7.0f) /* - */
		return -1.0f;
	else
		return input - 8.0f;
}

int main(int argc, char *argv[])
{
	float freq = 1000, amp = 0.5f, phase = 0.0f;
	float period;
	long len;
	long n;
	int i;

	get_rate();
	len = RATE;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-freq") && i+1 < argc)
			freq = atof(argv[++i]); /* frequency of the wave */
		else if (!strcmp(argv[i], "-amp") && i+1 < argc)
			amp = atof(argv[++i]);  /* amplitude from 0 to 1 */
		else if (!strcmp(argv[i], "-len") && i+1 < argc)
			len = atof(argv[++i]) / 1000.0f * RATE; /* length in ms */
		else if (!strcmp(argv[i], "-phase") && i+1 < argc)
			phase = atof(argv[++i]); /* phase in degrees */
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -freq arg, -amp arg, -len "
				"arg, -phase arg\n");
			exit(0);
		}
	}

	/* check options */
	freq  = CLAMP(0.01f, freq, RATE / 22.0f * 10.f);
	amp   = CLAMP(0.0f, amp, 1.0f);
	phase = CLAMP(0.0f, phase, 359.99f);

	/* convert options */
	period = RATE / freq;
	phase *= period;

	SET_BINARY_MODE
	for (n = 0; n < len; n++)
	{
		float f;

		f = octagon((n + phase) * 360.0f / period) * amp;

		/* left channel */
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			break;

		/* right channel */
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			break;
	}

	return 0;
}
