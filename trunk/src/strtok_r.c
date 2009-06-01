#include "headers.h"

char *strtok_r(char *s1, const char *s2, char **lasts)
{
	char *ret;

	if (s1 == NULL)
	{
		s1 = *lasts;
	}

	while (*s1 && strchr(s2, *s1))
	{
		++s1;
	}

	if (*s1 == '\0')
	{
		return NULL;
	}

	ret = s1;

	while (*s1 && !strchr(s2, *s1))
	{
		++s1;
	}

	if (*s1)
	{
		*s1++ = '\0';
	}

	*lasts = s1;

	return ret;
}
