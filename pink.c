#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary.h"

/* pink.c: make pink noise from white */

static void pink(void);

int main(void)
{
	SET_BINARY_MODE

	pink();
	return 0;
}

static void pink(void)
{
	float f;
	float b[14];
	float white, pink;
	int left = 0;
	int i;

	for (i = 0; i < 14; i++)
		b[i] = 0.0f;

	for (;;)
	{
		int o;
		left = !left;
		o = left ? 7 : 0;

		if (fread(&f, sizeof f, 1, stdin) < 1)
			return;

		white = f;

		b[0+o] = 0.99886 * b[0+o] + white * 0.0555179;
		b[1+o] = 0.99332 * b[1+o] + white * 0.0750759;
		b[2+o] = 0.96900 * b[2+o] + white * 0.1538520;
		b[3+o] = 0.86650 * b[3+o] + white * 0.3104856;
		b[4+o] = 0.55000 * b[4+o] + white * 0.5329522;
		b[5+o] = -0.7616 * b[5+o] - white * 0.0168980;
		pink = b[0+o] + b[1+o] + b[2+o] + b[3+o]
			+ b[4+o] + b[5+o] + b[6+o] + white * 0.5362;
		b[6+o] = white * 0.115926;

		f = pink;
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
