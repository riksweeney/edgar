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
		moveInventoryCursor(-INVENTORY_COLUMN_COUNT);

		input.up = FALSE;

		playSound("sound/common/click");
	}

	else if (input.down == TRUE)
	{
		moveInventoryCursor(INVENTORY_COLUMN_COUNT);

		input.down = FALSE;

		playSound("sound/common/click");
	}

	if (input.left == TRUE)
	{
		moveInventoryCursor(-1);

		input.left = FALSE;

		playSound("sound/common/click");
	}

	else if (input.right == TRUE)
	{
		moveInventoryCursor(1);

		input.right= FALSE;

		playSound("sound/common/click");
	}

	if (input.attack == TRUE || input.activate == TRUE)
	{
		input.attack = FALSE;

		input.activate = FALSE;

		selectInventoryItem();

		useInventoryItem();

		playSound("sound/common/click");
	}

	else if (input.block == TRUE || input.interact == TRUE)
	{
		input.block = FALSE;

		input.interact = FALSE;

		selectInventoryItem();

		playSound("sound/common/click");
	}

	else if (input.next == TRUE)
	{
		input.next = FALSE;

		moveInventoryItem(1);

		playSound("sound/common/click");
	}

	else if (input.previous == TRUE)
	{
		input.previous = FALSE;

		moveInventoryItem(-1);

		playSound("sound/common/click");
	}
}

void freeInventoryMenu()
{
	freeInventory();
}
