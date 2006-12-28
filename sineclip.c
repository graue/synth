#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* sineclip.c: sine-limit a waveform; a weird noise effect */

static void sineclip(float amp, float freq, float dc);

int main(int argc, char *argv[])
{
	float amp = 0.25f;
	float freq = 0.2f;
	float dc = 0.75f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && i+1 < argc)
			amp = atof(argv[++i]);
		else if (!strcmp(argv[i], "-freq") && i+1 < argc)
			freq = atof(argv[++i]);
		else if (!strcmp(argv[i], "-dc") && i+1 < argc)
			dc = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -amp amplitude, "
				"-freq frequency, -dc dc_minimum\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	sineclip(amp, freq, dc);
	return 0;
}

static void sineclip(float amp, float freq, float dc)
{
	float f[2]; /* 2 samples at once, left/right */
	float sinelimit;
	float phase = 0.0f;
	const float phase_inc = RATE / (2*M_PI*freq);

	while (fread(f, sizeof f, 1, stdin) == 1)
	{
		sinelimit = sin(phase)*amp + dc;
		f[0] = CLAMP(-sinelimit, f[0], sinelimit);
		f[1] = CLAMP(-sinelimit, f[1], sinelimit);

		if (fwrite(f, sizeof f, 1, stdout) < 1)
			return;

		phase += phase_inc;
	}
}
