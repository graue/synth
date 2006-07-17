#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "binary.h"
#include "defs.h"

/* white: generate white noise */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))

int main(int argc, char *argv[])
{
	float amp = 0.5f, range;
	long len = RATE;
	long n;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && ++i < argc)
			amp = atof(argv[i]);  /* amplitude from 0 to 1 */
		else if (!strcmp(argv[i], "-len") && ++i < argc)
			len = atof(argv[i]) / 1000.0f * RATE; /* length in ms */
	}

	/* check options */
	amp = CLAMP(0.0f, amp, 1.0f);

	/* convert options */
	amp *= 32768.0f;
	range = amp * 2;

	for (n = 0; n < len; n++)
	{
		float f;
		char *fr = (char *)&f;

		/* left channel */
		f = ((float)rand()) / RAND_MAX * range - amp;
		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fr[i]);

		/* right channel */
		f = ((float)rand()) / RAND_MAX * range - amp;
		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fr[i]);
	}

	return 0;
}
