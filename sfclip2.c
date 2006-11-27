#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* sfclip2.c: soft clipping using a tanh approximation */

static void sfclip2(void);

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
	sfclip2();
	return 0;
}

static void sfclip2(void)
{
	float f;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		f = f*(27 + f*f) / (27 + 9*f*f);
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
