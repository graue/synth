#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "synth.h"

/* power.c: raise each of a waveform's samples to a power */

static void power(float exponent, int signedexp);

int main(int argc, char *argv[])
{
	float exponent = 1.0f;
	int i;
	int signedexp = 0;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-exp") && i+1 < argc)
			exponent = atof(argv[++i]);
		else if (!strcmp(argv[i], "-signed"))
			signedexp = 1;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -exp exponent, -signed\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	power(exponent, signedexp);
	return 0;
}

static void power(float exponent, int signedexp)
{
	float f;
	int positiveexponent = exponent > 0.0f;
	int wholeexponent = exponent == floorf(exponent);

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		if (!signedexp)
		{
			if (positiveexponent || f != 0.0f)
			{
				if (f > 0.0f)
					f = pow(f, exponent);
				else
					f = -pow(-f, exponent);
			}
		}
		else if (!(!wholeexponent && f < 0.0f)
			&& !(!positiveexponent && f == 0.0f))
		{
			f = pow(f, exponent);
		}

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
