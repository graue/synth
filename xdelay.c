#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* xdelay.c: cross-delay */

static void xdelay(int leftlen, int rightlen, float feedback, float wetout);

int main(int argc, char *argv[])
{
	float leftlen = 50.0f, rightlen = 60.0f;
	float feedback = 37.5f;
	float wetout = 100.0f;
	int leftlen_smp, rightlen_smp;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-leftlen") && i+1 < argc)
			leftlen = atof(argv[++i]);
		else if (!strcmp(argv[i], "-rightlen") && i+1 < argc)
			rightlen = atof(argv[++i]);
		else if (!strcmp(argv[i], "-feedback") && i+1 < argc)
			feedback = atof(argv[++i]);
		else if (!strcmp(argv[i], "-wetout") && i+1 < argc)
			wetout = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -leftlen ms, -rightlen ms, "
				"-feedback percent, -wetout percent\n");
			exit(0);
		}
	}

	leftlen = CLAMP(0.0f, leftlen, 30000.0f);
	leftlen_smp = (int)(leftlen * RATE / 1000.0f);
	if (leftlen_smp == 0)
		leftlen_smp = 1;

	rightlen = CLAMP(0.0f, rightlen, 30000.0f);
	rightlen_smp = (int)(rightlen * RATE / 1000.0f);
	if (rightlen_smp == 0)
		rightlen_smp = 1;

	SET_BINARY_MODE
	xdelay(leftlen_smp, rightlen_smp, feedback, wetout);
	return 0;
}

static void xdelay(int leftlen_smp, int rightlen_smp,
	float feedback, float wetout)
{
	float *buf[2];
	float f[2], f0[2];
	const int len[2] = {leftlen_smp, rightlen_smp};
	int bufpos[2] = {0, 0};
	int i, j;

	/* convert percentages to ratios */
	feedback /= 100.0f;
	wetout /= 100.0f;

	for (i = 0; i < 2; i++)
	{
		buf[i] = malloc(sizeof *buf[0] * len[i]);
		if (buf[i] == NULL)
		{
			fprintf(stderr, "xdelay: out of memory\n");
			exit(EXIT_FAILURE);
		}
		for (j = 0; j < len[i]; j++)
			buf[i][j] = 0.0f;
	}

	while (fread(f0, sizeof f0[0], 2, stdin) == 2)
	{
		for (i = 0; i < 2; i++)
		{
			f[i] = f0[i] + wetout*buf[i][bufpos[i]];

			buf[i][bufpos[i]] = feedback*buf[i][bufpos[i]] + f0[i];
			if (++bufpos[i] == len[i])
				bufpos[i] = 0;
		}

		if (fwrite(f, sizeof f[0], 2, stdout) < 2)
			return;
	}

	free(buf[0]);
	free(buf[1]);
}
