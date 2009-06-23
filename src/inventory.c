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

#include "headers.h"

#include "graphics/animation.h"
#include "graphics/graphics.h"
#include "player.h"
#include "item/item.h"
#include "hud.h"
#include "event/trigger.h"
#include "event/global_trigger.h"
#include "system/properties.h"
#include "entity.h"
#include "dialog.h"

static Inventory inventory;
extern Entity *self;
extern Entity playerWeapon, playerShield;

static void sortInventory(void);

void freeInventory()
{
	/* Clear the list */

	memset(&inventory, 0, sizeof(Inventory) * MAX_INVENTORY_ITEMS);

	if (inventory.background != NULL)
	{
		SDL_FreeSurface(inventory.background);

		inventory.background = NULL;
	}
	
	if (inventory.description != NULL)
	{
		SDL_FreeSurface(inventory.description);

		inventory.description = NULL;
	}
	
	if (inventory.cursor != NULL)
	{
		SDL_FreeSurface(inventory.cursor);

		inventory.cursor = NULL;
	}
}

int addToInventory(Entity *e)
{
	int i, found;

	found = FALSE;

	if (e->flags & STACKABLE)
	{
		printf("Item is stackable\n");

		for (i=0;i<MAX_INVENTORY_ITEMS;i++)
		{
			if (strcmpignorecase(inventory.item[i].objectiveName, e->objectiveName) == 0)
			{
				printf("Stacking %s\n", e->objectiveName);

				inventory.item[i].health++;

				found = TRUE;

				break;
			}
		}
	}

	if (found == FALSE)
	{
		for (i=0;i<MAX_INVENTORY_ITEMS;i++)
		{
			if (inventory.item[i].inUse == FALSE)
			{
				inventory.item[i] = *e;

				inventory.item[i].face = RIGHT;

				inventory.item[i].thinkTime = 0;

				setEntityAnimation(&inventory.item[i], STAND);

				if (inventory.item[i].type == WEAPON)
				{
					autoSetPlayerWeapon(&inventory.item[i]);
				}

				else if (inventory.item[i].type == SHIELD)
				{
					autoSetPlayerShield(&inventory.item[i]);
				}

				found = TRUE;

				break;
			}
		}
	}

	if (found == TRUE)
	{
		e->inUse = FALSE;

		setInfoBoxMessage(120,  _("Picked up %s"), inventory.item[i].objectiveName);

		fireTrigger(inventory.item[i].objectiveName);

		fireGlobalTrigger(inventory.item[i].objectiveName);
	}

	return found;
}

void moveInventoryCursor(int index)
{
	inventory.cursorIndex += index;
	
	if (index == 1 && (inventory.cursorIndex % INVENTORY_BOX_COUNT) == 0)
	{
		inventory.cursorIndex -= INVENTORY_BOX_COUNT;
	}
	
	else if (index == -1 && (inventory.cursorIndex == -1 || (inventory.cursorIndex % INVENTORY_BOX_COUNT) == INVENTORY_BOX_COUNT - 1))
	{
		inventory.cursorIndex += INVENTORY_BOX_COUNT;
	}

	if (inventory.cursorIndex >= MAX_INVENTORY_ITEMS)
	{
		inventory.cursorIndex -= MAX_INVENTORY_ITEMS;
	}

	else if (inventory.cursorIndex < 0)
	{
		inventory.cursorIndex += MAX_INVENTORY_ITEMS;
	}
	
	if (inventory.description != NULL)
	{
		SDL_FreeSurface(inventory.description);

		inventory.description = NULL;
	}
}

void nextInventoryItem(int index)
{
	inventory.selectedIndex += index;
	
	while (inventory.item[inventory.selectedIndex].inUse == FALSE)
	{
		inventory.selectedIndex += index;
		
		if (inventory.selectedIndex >= MAX_INVENTORY_ITEMS)
		{
			inventory.selectedIndex = 0;
		}
	
		else if (inventory.selectedIndex < 0)
		{
			inventory.selectedIndex = MAX_INVENTORY_ITEMS - 1;
		}
	}
	
	if (inventory.description != NULL)
	{
		SDL_FreeSurface(inventory.description);

		inventory.description = NULL;
	}
}

void selectInventoryItem(void)
{
	if (inventory.item[inventory.cursorIndex].inUse == TRUE)
	{
		inventory.selectedIndex = inventory.cursorIndex;
	}
}

Entity *getInventoryItem(char *name)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].objectiveName, name) == 0)
		{
			return &inventory.item[i];
		}
	}

	return NULL;
}

int removeInventoryItem(char *name)
{
	int i, found;

	found = FALSE;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].objectiveName, name) == 0)
		{
			inventory.item[i].inUse = FALSE;

			found = TRUE;

			break;
		}
	}

	if (found == TRUE)
	{
		sortInventory();
	}

	return found;
}

void dropInventoryItem()
{
	if (inventory.item[inventory.selectedIndex].inUse == TRUE)
	{
		dropItem(&inventory.item[inventory.selectedIndex]);

		inventory.item[inventory.selectedIndex].inUse = FALSE;

		sortInventory();
	}
}

void useInventoryItem()
{
	int index;
	Entity *temp;

	if (inventory.item[inventory.selectedIndex].inUse == TRUE && inventory.item[inventory.selectedIndex].activate != NULL)
	{
		temp = self;
		
		index = inventory.selectedIndex;

		self = &inventory.item[inventory.selectedIndex];

		self->activate(0);

		if (inventory.item[inventory.selectedIndex].inUse == FALSE)
		{
			sortInventory();
			
			if (index == inventory.cursorIndex)
			{
				inventory.cursorIndex = inventory.selectedIndex;
			}
		}

		self = temp;
	}
}

static void sortInventory()
{
	int i, j;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == FALSE)
		{
			for (j=i;j<MAX_INVENTORY_ITEMS;j++)
			{
				if (inventory.item[j].inUse == TRUE)
				{
					inventory.item[i] = inventory.item[j];

					inventory.item[j].inUse = FALSE;

					break;
				}
			}
		}
	}

	while (inventory.item[inventory.selectedIndex].inUse == FALSE)
	{
		inventory.selectedIndex--;

		if (inventory.selectedIndex < 0)
		{
			inventory.selectedIndex = 0;

			break;
		}
	}
}

void doInventory()
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			inventory.item[i].thinkTime--;

			if (inventory.item[i].thinkTime < 0)
			{
				inventory.item[i].thinkTime = 0;
			}
		}
	}
}

void drawSelectedInventoryItem(int x, int y, int w, int h)
{
	self = &inventory.item[inventory.selectedIndex];

	if (self->inUse == TRUE)
	{
		drawLoopingAnimation(self, x, y, w, h, 1);
	}
}

void addRequiredToInventory(Entity *other)
{
	Entity *item;

	if (!(self->flags & INVULNERABLE) && other->type == PLAYER)
	{
		item = getInventoryItem(self->requires);

		if (item != NULL)
		{
			item->health++;

			self->inUse = FALSE;

			setInfoBoxMessage(120,  _("Picked up %s"), self->objectiveName);

			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);
		}

		else
		{
			setInfoBoxMessage(120, _("%s is required to carry this item"), self->requires);
		}
	}
}

void loadInventoryItems()
{
	int i, j;
	Entity e;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			loadProperties(inventory.item[i].name, &e);

			for (j=0;j<MAX_ANIMATION_TYPES;j++)
			{
				inventory.item[i].animation[j] = e.animation[j];
			}

			setEntityAnimation(&inventory.item[i], STAND);

			if (inventory.item[i].type == WEAPON && strcmpignorecase(inventory.item[i].name, playerWeapon.name) == 0)
			{
				self = &inventory.item[i];

				setPlayerWeapon(1);
			}

			else if (inventory.item[i].type == SHIELD && strcmpignorecase(inventory.item[i].name, playerShield.name) == 0)
			{
				self = &inventory.item[i];

				setPlayerShield(1);
			}
		}
	}

	printf("Done reloading inventory\n");
}

void getInventoryItemFromScript(char *line)
{
	char command[15], itemName[MAX_VALUE_LENGTH], entityName[MAX_VALUE_LENGTH];
	int quantity, success, failure, quantityToRemove;
	Entity *e, *item;

	sscanf(line, "%s \"%[^\"]\" %d %d %s %d %d", command, itemName, &quantity, &quantityToRemove, entityName, &success, &failure);

	e = getEntityByObjectiveName(entityName);

	if (e == NULL)
	{
		printf("Could not find Entity %s to give item %s to\n", entityName, itemName);

		exit(1);
	}

	item = getInventoryItem(itemName);

	if (item != NULL && (item->health >= quantity || quantity == 1))
	{
		if (strcmpignorecase(command, "REMOVE") == 0)
		{
			item->health -= quantityToRemove;

			if (item->health <= 0 || quantityToRemove == -1)
			{
				removeInventoryItem(itemName);
			}
		}

		e->health = success;
	}

	else
	{
		if (item == NULL)
		{
			printf("Could not find item %s\n", itemName);
		}

		else
		{
			printf("Only found %d of %d required\n", item->health, quantity);
		}

		e->health = failure;
	}
}

void writeInventoryToFile(FILE *fp)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		self = &inventory.item[i];

		if (self->inUse == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE %s\n", getEntityTypeByID(self->type));
			fprintf(fp, "NAME %s\n", self->name);
			fprintf(fp, "START_X %d\n", (int)self->x);
			fprintf(fp, "START_Y %d\n", (int)self->y);
			fprintf(fp, "END_X %d\n", (int)self->endX);
			fprintf(fp, "END_Y %d\n", (int)self->endY);
			fprintf(fp, "THINKTIME %d\n", self->thinkTime);
			fprintf(fp, "HEALTH %d\n", self->health);
			fprintf(fp, "DAMAGE %d\n", self->damage);
			fprintf(fp, "SPEED %0.1f\n", self->speed);
			fprintf(fp, "WEIGHT %0.2f\n", self->weight);
			fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
			fprintf(fp, "REQUIRES %s\n", self->requires);
			fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
			fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
			fprintf(fp, "}\n\n");
		}
	}
}

void drawInventory()
{
	int i, x, y;
	char description[MAX_MESSAGE_LENGTH];
	Entity *e;

	if (inventory.background == NULL)
	{
		inventory.background = createSurface(INVENTORY_BOX_SIZE * INVENTORY_BOX_COUNT, INVENTORY_BOX_SIZE * INVENTORY_BOX_COUNT);

		SDL_SetAlpha(inventory.background, SDL_SRCALPHA|SDL_RLEACCEL, 196);

		inventory.x = (SCREEN_WIDTH - inventory.background->w) / 2;
		inventory.y = (SCREEN_HEIGHT - inventory.background->h) / 2;
	}

	drawImage(inventory.background, inventory.x, inventory.y, FALSE);

	x = inventory.x;
	y = inventory.y;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			e = &inventory.item[i];

			drawLoopingAnimation(e, x, y, INVENTORY_BOX_SIZE, INVENTORY_BOX_SIZE, 1);
		}

		x += INVENTORY_BOX_SIZE;

		if (i != 0 && (i % INVENTORY_BOX_COUNT == 0))
		{
			y += INVENTORY_BOX_SIZE;

			x = inventory.x;
		}
	}
	
	e = &inventory.item[inventory.cursorIndex];
	
	if (inventory.description == NULL)
	{
		if (e->inUse == TRUE)
		{
			if (e->flags & STACKABLE)
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s (%d)", e->description, e->health);
			}
			
			else if (strlen(e->description) == 0 && strlen(e->objectiveName) != 0)
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s", e->objectiveName);
			}
			
			else
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s", e->description);
			}
		
			inventory.description = createDialogBox(NULL, description);
			
			drawImage(inventory.description, (SCREEN_WIDTH - inventory.description->w) / 2, inventory.y + inventory.background->h + 10, FALSE);
		}
	}
	
	else
	{
		drawImage(inventory.description, (SCREEN_WIDTH - inventory.description->w) / 2, inventory.y + inventory.background->h + 10, FALSE);
	}
	
	if (inventory.cursor == NULL)
	{
		inventory.cursor = loadImage("gfx/hud/inventory_cursor.png");
	}
	
	x = (inventory.cursorIndex % INVENTORY_BOX_COUNT) * INVENTORY_BOX_SIZE;
	y = (inventory.cursorIndex / INVENTORY_BOX_COUNT) * INVENTORY_BOX_SIZE;
	
	x += inventory.x;
	y += inventory.y;
	
	drawImage(inventory.cursor, x, y, FALSE);
}

void setInventoryDialogMessage(char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);
	
	if (inventory.description != NULL)
	{
		SDL_FreeSurface(inventory.description);
	}
	
	inventory.description = createDialogBox(NULL, text);
}
