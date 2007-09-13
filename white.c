#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "synth.h"
#define MT_IMPLEMENT
#include "mt.h"

/* white: generate white noise */

int main(int argc, char *argv[])
{
	float amp = 0.5f, range;
	long len = RATE;
	long n;
	int i;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && i+1 < argc)
			amp = atof(argv[++i]);  /* amplitude from 0 to 1 */
		else if (!strcmp(argv[i], "-len") && i+1 < argc)
			len = atof(argv[++i]) / 1000.0f * RATE; /* len in ms */
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -amp arg, -len arg\n");
			exit(0);
		}
	}

	/* check options */
	amp = CLAMP(0.0f, amp, 1.0f);

	/* convert options */
	range = amp * 2;

	mt_init((unsigned int)time(NULL));
	SET_BINARY_MODE
	for (n = 0; n < len*2; n++)
	{
		float f;

		f = mt_frand() * range - amp;
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			break;
	}

	return 0;
}
