#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* fix.c: fix NANs and infinities */

static void fix(void);

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
	fix();
	return 0;
}

static void fix(void)
{
	float f;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		if (isinff(f) || isnanf(f))
			f = 0.0f;
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
