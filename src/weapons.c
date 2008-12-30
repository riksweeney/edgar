#include "weapons.h"

extern void loadProperties(char *, Entity *);
extern void drawLoopingEntityAnimation(void);

void loadBasicWeapons()
{
	loadProperties("small_wooden_shield", &playerShield);
	loadProperties("basic_sword", &playerWeapon);
	
	playerShield.draw = &drawLoopingEntityAnimation;
	playerWeapon.draw = &drawLoopingEntityAnimation;
}
