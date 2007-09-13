#include <stdlib.h>

static int RATE = -1;

void get_rate(void)
{
	char *str;

	str = getenv("RATE");
	if (str == NULL)
		str = getenv("SR");

	if (str == NULL || atoi(str) <= 0)
		RATE = DEFAULT_RATE;
	else
		RATE = atoi(str);
}
