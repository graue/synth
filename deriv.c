#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* deriv.c: discretely differentiate */

static void deriv(void);

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "options: none\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	deriv();
	return 0;
}

static void deriv(void)
{
	float in[2], out[2];
	float last[2] = {0.0f, 0.0f};

	while (fread(in, sizeof in[0], 2, stdin) == 2)
	{
		out[0] = in[0] - 0.5*last[0];
		out[1] = in[1] - 0.5*last[1];

		if (fwrite(out, sizeof out[0], 2, stdout) < 1)
			return;

		last[0] = in[0];
		last[1] = in[1];
	}
}
