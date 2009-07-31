/*
Copyright (C) 2009 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../audio/audio.h"
#include "../inventory.h"

extern Input input;
extern Game game;
extern Control control;

void doInventoryMenu()
{
	if (input.up == TRUE)
	{
		moveInventoryCursor(-5);

		input.up = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.down == TRUE)
	{
		moveInventoryCursor(5);

		input.down = FALSE;

		playSound("sound/common/click.ogg");
	}

	if (input.left == TRUE)
	{
		moveInventoryCursor(-1);

		input.left = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.right == TRUE)
	{
		moveInventoryCursor(1);

		input.right= FALSE;

		playSound("sound/common/click.ogg");
	}

	if (input.attack == TRUE || input.activate == TRUE)
	{
		input.attack = FALSE;

		input.activate = FALSE;

		selectInventoryItem();

		useInventoryItem();

		playSound("sound/common/click.ogg");
	}

	else if (input.block == TRUE)
	{
		input.block = FALSE;

		selectInventoryItem();

		playSound("sound/common/click.ogg");
	}
}

void freeInventoryMenu()
{
	freeInventory();
}
