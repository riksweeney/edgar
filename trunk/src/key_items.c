#include "enemies.h"

extern void addChickenFeedBag(int, int);

static Special specials[] = {
{"chicken_feed_bag", &addChickenFeedBag}};

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
		}
	}
}
