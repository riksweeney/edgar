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

int getDistance(int x1, int y1, int x2, int y2)
{
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

void calculatePath(float startX, float startY, float endX, float endY, float *dirX, float *dirY)
{
	double x, y, t;

	x = endX - startX;
	y = endY - startY;

	t = fabs(x) + fabs(y);

	*dirX = t == 0 ? 0 : x / t;
	*dirY = t == 0 ? 0 : y / t;
}

void normalize(float *dirX, float *dirY)
{
	float normal;

	if (*dirX == 0 && *dirY == 0)
	{
		return;
	}

	normal = fabs(*dirX) > fabs(*dirY) ? fabs(*dirX) : fabs(*dirY);

	*dirX /= normal;
	*dirY /= normal;
}

void calculateTrajectory(int startX, int endX, float weight, float flightTime, float height, float *dirX, float *dirY)
{
	float hDistance;

	hDistance = abs(startX - endX);

	*dirX = hDistance / flightTime;

	if (startX > endX)
	{
		*dirX *= -1;
	}

	flightTime /= 2;

	*dirY = -(height + 0.5 * weight * GRAVITY_SPEED * flightTime * flightTime) / flightTime;
}

int getHorizontalDistance(Entity *e1, Entity *e2)
{
	int x1, x2;

	if (e1->x < e2->x)
	{
		x1 = e1->x + e1->box.x + e1->box.w - 1;

		x2 = e2->x + e2->w - 1 - e2->box.x - e2->box.w;
	}

	else
	{
		x1 = e1->x + e1->w - 1 - e1->box.x - e1->box.w;

		x2 = e2->x + e2->box.x + e2->box.w - 1;
	}

	return abs(x1 - x2);
}
