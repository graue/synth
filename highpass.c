#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"
#include "defs.h"

/* highpass.c: highpass filter */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void highpass(float cutoff, float q);

int main(int argc, char *argv[])
{
	float cutoff = RATE / 4.0f;
	float q = 1.0f;
	int i;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-cutoff") && ++i < argc)
		{
			cutoff = atof(argv[i]);
			cutoff = CLAMP(1.0f, cutoff, RATE * 0.499f);
		}
		else if (!strcmp(argv[i], "-q") && ++i < argc)
		{
			q = atof(argv[i]);
			q = CLAMP(0.01f, q, 100.0f);
		}
	}

	highpass(cutoff, q);
	return 0;
}

static void highpass(float cutoff, float q)
{
	float f;
	char *fw;
	int c, i, o = 0; /* o = odd */
	float w0, alpha;
	float b0, b1, b2, a0, a1, a2;
	float x[2][3] = {{0, 0, 0}, {0, 0, 0}};
	float y[2][3] = {{0, 0, 0}, {0, 0, 0}};
	const int n = 2;

	fw = (char *)&f;

	w0 = 2*M_PI * cutoff / RATE;
	alpha = sin(w0) / (2*q);

	b0 =  (1 + cos(w0))/2;
	b1 = -(1 + cos(w0));
	b2 =  (1 + cos(w0))/2;
	a0 =   1 + alpha;
	a1 =  -2*cos(w0);
	a2 =   1 - alpha;

	for (;;)
	{
		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = c;
		}

		o = !o;
		x[o][n-2] = x[o][n-1];
		x[o][n-1] = x[o][n];
		x[o][n] = f;
		y[o][n-2] = y[o][n-1];
		y[o][n-1] = y[o][n];
		y[o][n] = (b0/a0)*x[o][n] + (b1/a0)*x[o][n-1] + (b2/a0)*x[o][n-2]
		                          - (a1/a0)*y[o][n-1] - (a2/a0)*y[o][n-2];

		f = y[o][n];

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}
