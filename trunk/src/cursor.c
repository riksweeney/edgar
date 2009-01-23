#include "cursor.h"

extern void drawImage(SDL_Surface *, int, int);
extern void loadProperties(char *, Entity *);
extern void loadMap(char *);
extern void saveMap(void);
extern void setStatusMessage(char *);
extern void drawLoopingAnimationToMap(void);
extern Entity *getEntityAtLocation(int, int);
extern void mapStartXNext(int);
extern void mapStartYNext(int);
extern int nextTile(int);
extern int prevTile(int);
extern int mapStartX(void);
extern int mapStartY(void);
extern void setTileAt(int, int, int);
extern SDL_Surface *tileImage(int);
extern int isValidOnMap(Entity *);
extern void drawBox(int, int, int, int, int, int, int);
extern Entity *isSpaceEmpty(Entity *);
extern int addEntity(Entity, int, int);
extern void setPlayerLocation(int, int);
extern void centerMapOnEntity(Entity *);

static char *entityNames[] = {"edgar/edgar", "item/apple", "item/wooden_crate", "item/metal_crate",
							  "enemy/bat", "weapon/pickaxe",
							  "item/chicken_feed_bag", "enemy/chicken", 
							  "item/chicken_trap", "lift/mine_lift", "lift/lift_target"};
static int entityNamesLength = 0;

void initCursor()
{
	int i = 0;
	
	cursor.tileID = 0;
	cursor.entityType = 0;
	
	loadProperties(entityNames[0], &cursor.entity);
	
	cursor.entity.draw = &drawLoopingAnimationToMap;
	
	while (entityNames[i] != NULL)
	{
		i++;
	}
	
	entityNamesLength = i;
}

void doCursor()
{
	cursor.x = input.mouseX;
	cursor.y = input.mouseY;
	
	if (cursor.type == TILES)
	{
		cursor.x /= TILE_SIZE;
		cursor.y /= TILE_SIZE;
		
		cursor.x *= TILE_SIZE;
		cursor.y *= TILE_SIZE;
	}
	
	if (cursor.y >= SCREEN_HEIGHT - TILE_SIZE)
	{
		cursor.y = SCREEN_HEIGHT - TILE_SIZE * 2;
	}

	if (input.left == 1)
	{
		mapStartXNext(-TILE_SIZE);
	}
	
	else if (input.right == 1)
	{
		mapStartXNext(TILE_SIZE);
	}
	
	if (input.up == 1)
	{
		mapStartYNext(-TILE_SIZE);
	}
	
	else if (input.down == 1)
	{
		mapStartYNext(TILE_SIZE);
	}
	
	if (input.add == 1)
	{
		if (cursor.type == TILES)
		{
			setTileAt((mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE, cursor.tileID);
		}
		
		else
		{
			/* Entities can only be placed in blank spaces */
			
			if (isValidOnMap(&cursor.entity) == 1 && isSpaceEmpty(&cursor.entity) == NULL)
			{
				if (cursor.entityType == 0)
				{
					setPlayerLocation(mapStartX() + cursor.x, mapStartY() + cursor.y);
				}
				
				else
				{
					addEntity(cursor.entity, mapStartX() + cursor.x, mapStartY() + cursor.y);
				}
			}
			
			input.add = 0;
		}
	}
	
	else if (input.remove == 1)
	{
		if (cursor.type == TILES)
		{
			setTileAt((mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE, BLANK_TILE);
		}
		
		else
		{
			self = isSpaceEmpty(&cursor.entity);
			
			if (self != NULL)
			{
				self->active = INACTIVE;
			}
		}
	}
	
	if (input.previous == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.tileID = prevTile(cursor.tileID);
		}
		
		else
		{
			cursor.entityType--;
			
			if (cursor.entityType < 0)
			{
				cursor.entityType = entityNamesLength - 1;
			}
			
			loadProperties(entityNames[cursor.entityType], &cursor.entity);
		}
		
		input.previous = 0;
	}
	
	else if (input.next == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.tileID = nextTile(cursor.tileID);
		}
		
		else
		{
			cursor.entityType++;
			
			if (cursor.entityType >= entityNamesLength)
			{
				cursor.entityType = 0;
			}
			
			loadProperties(entityNames[cursor.entityType], &cursor.entity);
		}
		
		input.next = 0;
	}
	
	if (input.save == 1)
	{
		saveMap();
		
		printf("Saved\n");
		
		input.save = 0;
	}
	/*
	if (input.load == 1)
	{
		loadMap(map.filename);
		
		printf("Loaded\n");
		
		input.load = 0;
	}
	*/
	if (input.left == 1 || input.right == 1 || input.up == 1 || input.down == 1)
	{
		SDL_Delay(30);
	}
	
	if (input.toggle == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.type = ENTITIES;
		}
		
		else
		{
			cursor.type = TILES;
		}
		
		input.toggle = 0;
	}
	
	centerMapOnEntity(NULL);
}

void drawCursor()
{
	if (cursor.type == TILES)
	{
		drawImage(tileImage(cursor.tileID), cursor.x, cursor.y);
		drawImage(tileImage(BLANK_TILE), cursor.x, cursor.y);
	}
	
	else
	{		
		if (isValidOnMap(&cursor.entity) == 0 || isSpaceEmpty(&cursor.entity) != NULL)
		{
			drawBox(cursor.x, cursor.y, cursor.entity.w, cursor.entity.h, 255, 0, 0);
		}
		
		cursor.entity.x = mapStartX() + cursor.x;
		cursor.entity.y = mapStartY() + cursor.y;
		
		self = &cursor.entity;
		
		self->draw();
	}
}
