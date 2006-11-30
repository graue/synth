#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* sine: generate a sine wave */

int main(int argc, char *argv[])
{
	float freq = 1000.0f, amp = 0.5f, startphase = 0.0f;
	double phase, inc;
	long len = RATE;
	long n;
	int i;
	int modfreq = 0; /* use input to modulate frequency */

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
			startphase = atof(argv[i]); /* phase in degrees */
		else if (!strcmp(argv[i], "-modfreq"))
			modfreq = 1; /* modulate frequency with input */
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -freq arg, -amp arg, -len "
				"arg, -phase arg, -modfreq\n");
			exit(0);
		}
	}

	/* check options */
	freq = CLAMP(0.01f, freq, RATE / 22.0f * 10.f);
	amp = CLAMP(0.0f, amp, 1.0f);
	startphase = CLAMP(0.0f, startphase, 359.99f);

	/* convert options */
	amp *= 32768.0f;
	startphase = startphase * M_PI / 180.0f;

	phase = startphase;
	inc = 2*M_PI * freq / RATE;

	SET_BINARY_MODE
	for (n = 0; n < len; n++)
	{
		float f;

		if (modfreq)
		{
			float inputs[2], avg;
			if (fread(inputs, sizeof inputs[0], 2, stdin) < 2)
				avg = 0.0f;
			else
				avg = (inputs[0] + inputs[1] / 2) / 32768.0f;
			phase += avg;
		}

		f = amp * sin(phase);
		phase += inc;

		/* left channel */
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			break;

		/* right channel */
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			break;
	}

	return 0;
}
