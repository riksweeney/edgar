/*
Copyright (C) 2009-2019 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

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
