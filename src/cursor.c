#include "cursor.h"

extern void drawImage(SDL_Surface *, int, int);
extern void loadProperties(char *, Entity *);
extern void loadMap(char *);
extern void saveMap(void);
extern void setStatusMessage(char *);
extern void drawLoopingEntityAnimation(void);
extern Entity *getEntityAtLocation(int, int);
extern void mapStartXNext(int);
extern void mapStartYNext(int);
extern int nextTile(int);
extern int prevTile(int);
extern int mapStartX(void);
extern int mapStartY(void);
extern void setTileAt(int, int, int);
extern SDL_Surface *tileImage(int);

void initCursor()
{
	cursor.tileID = 0;
	cursor.entityType = 0;
	
	cursor.entity.draw = &drawLoopingEntityAnimation;
}

void doCursor()
{
	cursor.x = input.mouseX;
	cursor.y = input.mouseY;
	
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
			self = getEntityAtLocation((mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE);
			
			/* Don't allow a tile to be placed on top of an Entity */
			
			if (self == NULL)
			{
				printf("Placing tile %d to %d %d\n", cursor.tileID, (mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE);
				setTileAt((mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE, cursor.tileID);
			}
		}
		
		else
		{
			/* Entities can only be placed in blank spaces */
			/*
			if (map.tile[(map.startY + cursor.y) / TILE_SIZE][(map.startX + cursor.x) / TILE_SIZE] == BLANK_TILE)
			{
				self = getEntityAtLocation((map.startX + cursor.x) / TILE_SIZE, (map.startY + cursor.y) / TILE_SIZE);
				
				if (self != NULL)
				{
					self->active = INACTIVE;
				}
				
				switch (cursor.entityType)
				{
					case SPANNER:
						addSpanner((map.startX + cursor.x) / TILE_SIZE, (map.startY + cursor.y) / TILE_SIZE);
					break;
					
					default:
						
					break;
				}
			}
			*/
			input.add = 0;
		}
	}
	
	else if (input.remove == 1)
	{
		if (cursor.type == TILES)
		{
			setTileAt(BLANK_TILE, (mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE);
		}
		
		else
		{
			/*
			self = getEntityAtLocation((map.startX + cursor.x) / TILE_SIZE, (map.startY + cursor.y) / TILE_SIZE);
			
			if (self != NULL)
			{
				self->active = ACTIVE;
			}
			*/
			input.remove = 0;
		}
	}
	
	if (cursor.type == TILES)
	{
		if (input.previous == 1)
		{
			cursor.tileID = prevTile(cursor.tileID);
			
			input.previous = 0;
		}
		
		if (input.next == 1)
		{
			cursor.tileID = nextTile(cursor.tileID);
			
			input.next = 0;
		}
	}
	
	else
	{
		/*
		if (input.previous == 1)
		{
			cursor.entityType--;
			
			if (cursor.entityType < 0)
			{
				cursor.entityType = MAX_ENTITY_TYPES - 1;
			}
			
			input.previous = 0;
			
			switch (cursor.entityType)
			{
				case SPANNER:
					setAnimation("SPANNER", &cursor.entity);
				break;
				
				default:
					printf("Could not find Entity type!\n");
					
					exit(1);
			}
		}
		
		if (input.next == 1)
		{
			cursor.entityType++;
			
			if (cursor.entityType >= MAX_ENTITY_TYPES)
			{
				cursor.entityType = 0;
			}
			
			input.next = 0;
			
			switch (cursor.entityType)
			{
				case SPANNER:
					setAnimation("SPANNER", &cursor.entity);
				break;
				
				default:
					printf("Could not find Entity type!\n");
					
					exit(1);
			}
		}
		*/
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
	/*
	if (input.toggle == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.type = ENTITIES;
			
			cursor.entityType = SPANNER;
			
			setAnimation("SPANNER", &cursor.entity);
		}
		
		else
		{
			cursor.type = TILES;
			
			cursor.tileID = 0;
		}
		
		input.toggle = 0;
	}
	*/
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
		/*
		drawImage(mapImages[BLANK_TILE], cursor.x, cursor.y);
		
		cursor.entity.x = map.startX + cursor.x + (TILE_SIZE - cursor.entity.w) / 2;
		cursor.entity.y = map.startY + cursor.y + (TILE_SIZE - cursor.entity.h) / 2;
		
		self = &cursor.entity;
		
		self->draw();
		*/
	}
}
