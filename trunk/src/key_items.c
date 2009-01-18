#include "key_items.h"

extern void addChickenFeedBag(int, int);

static Special specials[] = {
{"item/chicken_feed_bag", &addChickenFeedBag}};

static int length = sizeof(specials) / sizeof(Special);

void addKeyItem(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(specials[i].name, name) == 0)
		{
			printf("Adding Key Item %s to %d %d\n", specials[i].name, x, y);

			specials[i].construct(x, y);
			
			return;
		}
	}
	
	printf("Could not find key item %s\n", name);
	
	exit(1);
}
