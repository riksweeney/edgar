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

#include "dialog.h"
#include "entity.h"
#include "event/global_trigger.h"
#include "event/trigger.h"
#include "graphics/animation.h"
#include "graphics/graphics.h"
#include "hud.h"
#include "inventory.h"
#include "item/item.h"
#include "item/key_items.h"
#include "player.h"
#include "system/error.h"
#include "system/properties.h"

static Inventory inventory;
extern Entity *self;
extern Entity playerWeapon, playerShield;

static void sortInventory(void);

void freeInventory()
{
	/* Clear the list */

	memset(inventory.item, 0, sizeof(Entity) * MAX_INVENTORY_ITEMS);

	inventory.selectedIndex = 0;

	inventory.cursorIndex = 0;

	inventory.hasLightningSword = FALSE;

	inventory.hasBow = FALSE;

	if (inventory.background != NULL)
	{
		destroyTexture(inventory.background);

		inventory.background = NULL;
	}

	inventory.description = NULL;

	if (inventory.cursor != NULL)
	{
		destroyTexture(inventory.cursor);

		inventory.cursor = NULL;
	}

	inventory.objectives = NULL;
}

int addToInventory(Entity *e)
{
	int i, found;
	Entity *temp;

	found = FALSE;

	if (e->flags & STACKABLE)
	{
		for (i=0;i<MAX_INVENTORY_ITEMS;i++)
		{
			if (strcmpignorecase(inventory.item[i].objectiveName, e->objectiveName) == 0 && inventory.item[i].inUse == TRUE)
			{
				if (inventory.item[i].health < MAX_STACKABLES)
				{
					inventory.item[i].health += (e->health == 0 ? 1 : e->health);

					if (inventory.item[i].health > MAX_STACKABLES)
					{
						inventory.item[i].health = MAX_STACKABLES;
					}

					found = TRUE;

					break;
				}

				else
				{
					setInfoBoxMessage(0, 255, 255, 255, _("Cannot carry any more %s"), _(inventory.item[i].objectiveName));

					return FALSE;
				}
			}
		}
	}

	if (found == FALSE)
	{
		for (i=0;i<MAX_INVENTORY_ITEMS;i++)
		{
			if (inventory.item[i].inUse == FALSE)
			{
				if (e->action == &generalItemAction)
				{
					e->action = &doNothing;
				}

				if (strcmpignorecase(e->name, "item/safe_combination") == 0)
				{
					temp = self;

					self = e;

					self->activate(1);

					self = temp;
				}

				e->flags &= ~DO_NOT_PERSIST;

				inventory.item[i] = *e;

				inventory.item[i].face = RIGHT;

				inventory.item[i].thinkTime = 0;

				setEntityAnimationByID(&inventory.item[i], 0);

				if (inventory.item[i].type == WEAPON)
				{
					if (strcmpignorecase(inventory.item[i].name, "weapon/bow") == 0)
					{
						inventory.hasBow = TRUE;
					}

					else if (strcmpignorecase(inventory.item[i].name, "weapon/lightning_sword") == 0)
					{
						inventory.hasLightningSword = TRUE;
					}

					else if (strcmpignorecase(inventory.item[i].name, "weapon/lightning_sword_empty") == 0)
					{
						inventory.hasLightningSword = TRUE;
					}

					if (strcmpignorecase(inventory.item[i].name, "weapon/normal_arrow") != 0 &&
						strcmpignorecase(inventory.item[i].name, "weapon/flaming_arrow") != 0)
					{
						autoSetPlayerWeapon(&inventory.item[i]);
					}
				}

				else if (inventory.item[i].type == SHIELD)
				{
					if (strcmpignorecase(inventory.item[i].name, "weapon/disintegration_shield") == 0)
					{
						switch (inventory.item[i].health)
						{
							case 1:
								loadProperties("weapon/disintegration_shield_1", &inventory.item[i]);
							break;

							case 2:
								loadProperties("weapon/disintegration_shield_2", &inventory.item[i]);
							break;

							case 3:
								loadProperties("weapon/disintegration_shield_3", &inventory.item[i]);
							break;

							case 4:
								loadProperties("weapon/disintegration_shield_4", &inventory.item[i]);
							break;

							default:
								loadProperties("weapon/disintegration_shield", &inventory.item[i]);
							break;
						}
					}

					autoSetPlayerShield(&inventory.item[i]);
				}

				if ((inventory.item[i].flags & STACKABLE) && inventory.item[i].health == 0)
				{
					inventory.item[i].health = 1;
				}

				found = TRUE;

				break;
			}
		}
	}

	if (found == TRUE)
	{
		if ((inventory.item[i].flags & STACKABLE) && e->health > 1)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Picked up %s x %d"), _(inventory.item[i].objectiveName), e->health);
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Picked up %s"), _(inventory.item[i].objectiveName));
		}

		e->inUse = FALSE;

		fireTrigger(inventory.item[i].objectiveName);

		fireGlobalTrigger(inventory.item[i].objectiveName);
	}

	else
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Cannot pick up %s. Inventory full"), _(e->objectiveName));

		return FALSE;
	}

	return found;
}

void moveInventoryCursor(int index)
{
	inventory.cursorIndex += index;

	if (index == 1 && (inventory.cursorIndex % INVENTORY_COLUMN_COUNT) == 0)
	{
		inventory.cursorIndex -= INVENTORY_COLUMN_COUNT;
	}

	else if (index == -1 && (inventory.cursorIndex == -1 || (inventory.cursorIndex % INVENTORY_COLUMN_COUNT) == INVENTORY_COLUMN_COUNT - 1))
	{
		inventory.cursorIndex += INVENTORY_COLUMN_COUNT;
	}

	if (inventory.cursorIndex >= MAX_INVENTORY_ITEMS)
	{
		inventory.cursorIndex -= MAX_INVENTORY_ITEMS;
	}

	else if (inventory.cursorIndex < 0)
	{
		inventory.cursorIndex += MAX_INVENTORY_ITEMS;
	}

	inventory.description = NULL;
}

void nextInventoryItem(int index)
{
	int currentIndex = inventory.selectedIndex;

	inventory.selectedIndex += index;

	while (inventory.item[inventory.selectedIndex].inUse == FALSE)
	{
		if (inventory.selectedIndex == currentIndex)
		{
			break;
		}

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

	inventory.cursorIndex = inventory.selectedIndex;

	inventory.description = NULL;
}

void moveInventoryItem(int index)
{
	Entity temp;

	if (inventory.item[inventory.cursorIndex].inUse == TRUE)
	{
		if (index == 1 && inventory.item[inventory.cursorIndex + index].inUse == TRUE && inventory.cursorIndex + index < MAX_INVENTORY_ITEMS)
		{
			temp = inventory.item[inventory.cursorIndex + index];

			inventory.item[inventory.cursorIndex + index] = inventory.item[inventory.cursorIndex];

			inventory.item[inventory.cursorIndex] = temp;

			moveInventoryCursor(index);

			if (inventory.cursorIndex % INVENTORY_COLUMN_COUNT == 0)
			{
				moveInventoryCursor(INVENTORY_COLUMN_COUNT);
			}
		}

		else if (index == -1 && inventory.cursorIndex + index >= 0)
		{
			temp = inventory.item[inventory.cursorIndex + index];

			inventory.item[inventory.cursorIndex + index] = inventory.item[inventory.cursorIndex];

			inventory.item[inventory.cursorIndex] = temp;

			moveInventoryCursor(index);

			if (inventory.cursorIndex != 0 && (inventory.cursorIndex % INVENTORY_COLUMN_COUNT) == INVENTORY_COLUMN_COUNT - 1)
			{
				moveInventoryCursor(-INVENTORY_COLUMN_COUNT);
			}
		}
	}
}

void selectInventoryItem()
{
	if (inventory.item[inventory.cursorIndex].inUse == TRUE)
	{
		inventory.selectedIndex = inventory.cursorIndex;

		setInventoryDialogMessage(_("Selected %s"), _(inventory.item[inventory.cursorIndex].objectiveName));
	}
}

Entity *getInventoryItemByObjectiveName(char *name)
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

Entity *getInventoryItemByName(char *name)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].name, name) == 0)
		{
			return &inventory.item[i];
		}
	}

	return NULL;
}

Entity *getCurrentInventoryItem()
{
	return inventory.item[inventory.selectedIndex].inUse == TRUE ? &inventory.item[inventory.selectedIndex] : NULL;
}

void replaceInventoryItem(char *name, Entity *e)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].objectiveName, name) == 0)
		{
			inventory.item[i] = *e;

			inventory.item[i].face = RIGHT;

			inventory.item[i].thinkTime = 0;

			setEntityAnimationByID(&inventory.item[i], 0);

			return;
		}
	}

	showErrorAndExit("Could not find inventory item %s to replace", name);
}

void replaceInventoryItemWithName(char *name, Entity *e)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].name, name) == 0)
		{
			inventory.item[i] = *e;

			inventory.item[i].face = RIGHT;

			inventory.item[i].thinkTime = 0;

			inventory.item[i].alpha = 255;

			setEntityAnimationByID(&inventory.item[i], 0);

			return;
		}
	}

	showErrorAndExit("Could not find inventory item %s to replace", name);
}

int removeInventoryItemByObjectiveName(char *name)
{
	int i, found;
	Entity *e;

	found = FALSE;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].objectiveName, name) == 0)
		{
			if (inventory.item[i].flags & STACKABLE)
			{
				inventory.item[i].health--;

				if (inventory.item[i].health <= 0)
				{
					inventory.item[i].inUse = FALSE;
				}
			}

			else
			{
				inventory.item[i].inUse = FALSE;

				if (strcmpignorecase(playerWeapon.name, inventory.item[i].name) == 0)
				{
					e = removePlayerWeapon();

					e->inUse = FALSE;
				}

				else if (strcmpignorecase(playerShield.name, inventory.item[i].name) == 0)
				{
					e = removePlayerShield();

					e->inUse = FALSE;
				}
			}

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

int removeInventoryItemByName(char *name)
{
	int i, found;
	Entity *e;

	found = FALSE;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].name, name) == 0)
		{
			inventory.item[i].inUse = FALSE;

			if (strcmpignorecase(playerWeapon.name, inventory.item[i].name) == 0)
			{
				e = removePlayerWeapon();

				e->inUse = FALSE;
			}

			else if (strcmpignorecase(playerShield.name, inventory.item[i].name) == 0)
			{
				e = removePlayerShield();

				e->inUse = FALSE;
			}

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
		item = getInventoryItemByObjectiveName(self->requires);

		if (item != NULL)
		{
			item->health++;

			self->inUse = FALSE;

			setInfoBoxMessage(60, 255, 255, 255, _("Picked up %s"), _(self->objectiveName));

			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("%s is required to carry this item"), _(self->requires));
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

			inventory.item[i].fallout = &keyItemFallout;

			for (j=0;j<MAX_ANIMATION_TYPES;j++)
			{
				inventory.item[i].animation[j] = e.animation[j];
			}

			inventory.item[i].currentAnim = -1;

			setEntityAnimationByID(&inventory.item[i], 0);

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

			else if (inventory.item[i].type == TEMP_ITEM)
			{
				inventory.item[i].type = ITEM;

				inventory.item[i].action = &doNothing;

				inventory.item[i].flags &= ~DO_NOT_PERSIST;
			}
		}
	}
}

void getInventoryItemFromScript(char *line)
{
	char command[15], itemName[MAX_VALUE_LENGTH], entityName[MAX_VALUE_LENGTH], quiet[MAX_VALUE_LENGTH];
	int quantity, success, failure, quantityToRemove, read;
	Entity *e, *item;

	read = sscanf(line, "%s \"%[^\"]\" %d %d %s %d %d %s", command, itemName, &quantity, &quantityToRemove, entityName, &success, &failure, quiet);

	if (read < 7)
	{
		showErrorAndExit("HAS_ITEM or REMOVE command has wrong number of arguments");
	}

	e = getEntityByObjectiveName(entityName);

	if (e == NULL)
	{
		showErrorAndExit("Could not find Entity %s to give item %s to", entityName, itemName);
	}

	item = getInventoryItemByObjectiveName(itemName);

	if (item != NULL && (item->health >= quantity || quantity == 1))
	{
		if (strcmpignorecase(command, "REMOVE") == 0)
		{
			item->health -= quantityToRemove;

			updateTrigger(itemName, quantityToRemove);

			updateGlobalTrigger(itemName, quantityToRemove);

			if (item->health <= 0 || quantityToRemove == -1)
			{
				item->health = 0;

				removeInventoryItemByObjectiveName(itemName);

				if (read == 7)
				{
					setInfoBoxMessage(90, 255, 255, 255, _("Removed %s"), _(itemName));
				}
			}
		}

		e->health = success;
	}

	else
	{
		e->health = failure;
	}
}

void useInventoryItemFromScript(char *itemName)
{
	Entity *item, *temp;

	item = getInventoryItemByObjectiveName(itemName);

	if (item == NULL)
	{
		showErrorAndExit("Could not find Inventory Item %s", itemName);
	}

	temp = self;

	self = item;

	self->activate(0);

	self = temp;
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
			fprintf(fp, "MENTAL %d\n", self->mental);
			fprintf(fp, "HEALTH %d\n", self->health);
			fprintf(fp, "DAMAGE %d\n", self->damage);
			fprintf(fp, "SPEED %0.2f\n", self->speed);
			fprintf(fp, "WEIGHT %0.2f\n", self->weight);
			fprintf(fp, "OBJECTIVE_NAME %s\n", self->objectiveName);
			fprintf(fp, "REQUIRES %s\n", self->requires);
			fprintf(fp, "ACTIVE %s\n", self->active == TRUE ? "TRUE" : "FALSE");
			fprintf(fp, "FACE %s\n", self->face == RIGHT ? "RIGHT" : "LEFT");
			fprintf(fp, "}\n\n");
		}
	}
}

void clearInventoryDescription()
{
	inventory.description = NULL;

	inventory.objectives = NULL;
}

void drawInventory()
{
	int i, x, y, descriptionY;
	char description[MAX_MESSAGE_LENGTH];
	Entity *e;

	if (inventory.background == NULL)
	{
		inventory.background = createTexture(INVENTORY_BOX_SIZE * INVENTORY_COLUMN_COUNT, INVENTORY_BOX_SIZE * INVENTORY_ROW_COUNT, 0, 0, 0);

		inventory.x = (SCREEN_WIDTH - inventory.background->w) / 2;
		inventory.y = 64;
	}

	drawImage(inventory.background, inventory.x, inventory.y, FALSE, 196);

	x = inventory.x;
	y = inventory.y;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			e = &inventory.item[i];

			if (inventory.selectedIndex == i)
			{
				drawSprite(e, x, y, INVENTORY_BOX_SIZE, INVENTORY_BOX_SIZE, 1);
			}

			else
			{
				drawLoopingAnimation(e, x, y, INVENTORY_BOX_SIZE, INVENTORY_BOX_SIZE, 1);
			}
		}

		x += INVENTORY_BOX_SIZE;

		if (i != 0 && ((i + 1) % INVENTORY_COLUMN_COUNT == 0))
		{
			y += INVENTORY_BOX_SIZE;

			x = inventory.x;
		}
	}

	e = &inventory.item[inventory.cursorIndex];

	descriptionY = inventory.y + inventory.background->h + 10;

	if (inventory.description == NULL)
	{
		if (e->inUse == TRUE)
		{
			if (e->flags & STACKABLE)
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s (%d)", _(e->description), e->health);
			}

			else if (strcmpignorecase(e->name, "weapon/lightning_sword") == 0)
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s (%d)", _(e->objectiveName), e->mental);
			}

			else if (strlen(e->description) == 0 && strlen(e->objectiveName) != 0)
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s", _(e->objectiveName));
			}

			else
			{
				snprintf(description, MAX_MESSAGE_LENGTH, "%s", _(e->description));
			}

			inventory.description = createDialogBox(NULL, description);

			drawImage(inventory.description, (SCREEN_WIDTH - inventory.description->w) / 2, descriptionY, FALSE, 255);
		}
	}

	else
	{
		drawImage(inventory.description, (SCREEN_WIDTH - inventory.description->w) / 2, descriptionY, FALSE, 255);
	}

	if (inventory.description != NULL)
	{
		descriptionY += inventory.description->h + 10;
	}

	if (inventory.objectives == NULL)
	{
		inventory.objectives = listObjectives();
	}

	if (inventory.cursor == NULL)
	{
		inventory.cursor = loadImage("gfx/hud/inventory_cursor.png");
	}

	x = (inventory.cursorIndex % INVENTORY_COLUMN_COUNT) * INVENTORY_BOX_SIZE;
	y = (inventory.cursorIndex / INVENTORY_COLUMN_COUNT) * INVENTORY_BOX_SIZE;

	x += inventory.x;
	y += inventory.y;

	drawImage(inventory.cursor, x, y, FALSE, 255);

	drawImage(inventory.objectives, (SCREEN_WIDTH - inventory.objectives->w) / 2, descriptionY, FALSE, 255);
}

void setInventoryDialogMessage(char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	inventory.description = createDialogBox(NULL, text);
}

void setInventoryIndex(int val)
{
	inventory.selectedIndex = val;

	moveInventoryCursor(val);
}

int getInventoryIndex()
{
	return inventory.selectedIndex;
}

int hasBow()
{
	return inventory.hasBow;
}

int hasLightningSword()
{
	return inventory.hasLightningSword;
}

void scriptAddToInventory(char *name, int quiet)
{
	int i;
	Entity *e;

	e = addPermanentItem(name, 0, 0);

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == FALSE)
		{
			inventory.item[i] = *e;

			inventory.item[i].face = RIGHT;

			inventory.item[i].thinkTime = 0;

			setEntityAnimationByID(&inventory.item[i], 0);

			if (quiet == FALSE)
			{
				setInfoBoxMessage(60, 255, 255, 255, _("Picked up %s"), _(inventory.item[i].objectiveName));
			}

			break;
		}
	}

	e->inUse = FALSE;
}

Entity *removeInventoryItemAtCursor()
{
	sortInventory();

	return (inventory.item[inventory.selectedIndex].inUse == TRUE ? &inventory.item[inventory.selectedIndex] : NULL);
}

void resetInventoryIndex()
{
	inventory.selectedIndex = 0;

	inventory.cursorIndex = 0;
}

