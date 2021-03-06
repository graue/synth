#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* midside.c: convert left/right to mid/side */

/*
 * Note: This program also undoes itself, but it has a side effect of
 * ~6 dB gain in the process (2x amplitude).
 */

static void midside(void);

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: none\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	midside();
	return 0;
}

static void midside(void)
{
	float in[2], out[2];

	while (fread(in, sizeof in[0], 2, stdin) == 2)
	{
		out[0] = in[0] + in[1];
		out[1] = in[0] - in[1];

		if (fwrite(out, sizeof out[0], 2, stdout) < 1)
			return;
	}
}
