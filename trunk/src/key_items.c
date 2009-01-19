#include "key_items.h"

extern void addChickenFeedBag(int, int);
extern void addChickenTrap(int, int);

static Constructor items[] = {
{"item/chicken_feed_bag", &addChickenFeedBag},
{"item/chicken_trap", &addChickenTrap}
};

static int length = sizeof(items) / sizeof(Constructor);

void addKeyItem(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(items[i].name, name) == 0)
		{
			printf("Adding Key Item %s to %d %d\n", items[i].name, x, y);

			items[i].construct(x, y);
			
			return;
		}
	}
	
	printf("Could not find key item %s\n", name);
	
	exit(1);
}
