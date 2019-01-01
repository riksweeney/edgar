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

Entity *addPermanentItem(char *, int, int);
Entity *addTemporaryItem(char *, int, int, int, float, float);
void dropRandomItem(int, int);
void generalItemAction(void);
void healthTouch(Entity *);
void lightningChargeTouch(Entity *);
void throwItem(int);
Entity *dropCollectableItem(char *, int, int, int);
void itemFallout(void);
