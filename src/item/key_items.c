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

#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../inventory.h"
#include "../system/error.h"
#include "item.h"
#include "key_items.h"

#include "chicken_feed.h"
#include "chicken_trap.h"
#include "coal_pile.h"
#include "heart_container.h"
#include "shrub.h"
#include "stalactite.h"
#include "bomb.h"
#include "bomb_pile.h"
#include "potions.h"
#include "power_generator.h"
#include "grabber.h"
#include "arrow_target.h"
#include "symbol_block.h"
#include "force_field.h"
#include "gazer_eye_slot.h"
#include "spring.h"
#include "number_block.h"
#include "bubble_machine.h"
#include "bubble.h"
#include "potion_dispenser.h"
#include "tesla_charger.h"
#include "tesla_pack.h"
#include "high_striker.h"
#include "containment_unit.h"
#include "containment_unit_controls.h"
#include "ice_spray.h"
#include "blending_machine.h"
#include "spike_ball.h"
#include "code_door.h"
#include "code_display.h"
#include "skull.h"
#include "glass_wall.h"
#include "robot.h"
#include "tuning_fork.h"
#include "pedestal.h"
#include "cauldron.h"
#include "soul_bottle.h"
#include "instruction_machine.h"
#include "moveable_lift.h"
#include "mine_cart.h"
#include "rock_container.h"
#include "scale.h"
#include "lava_geyzer.h"
#include "crusher.h"
#include "repellent.h"
#include "cloud_geyzer.h"
#include "slime_container.h"
#include "final_power_generator.h"
#include "mirror.h"
#include "light_source.h"
#include "light_target.h"
#include "balloon.h"
#include "water_purifier.h"
#include "extend_o_grab.h"
#include "sliding_puzzle.h"
#include "sliding_puzzle_display.h"
#include "tread_mill.h"
#include "power_meter.h"
#include "shield_pillar.h"
#include "crossbow.h"
#include "crossbow_bolt.h"
#include "catapult.h"
#include "light_pillar.h"
#include "exploding_gazer_eye.h"
#include "exploding_gazer_eye_dud.h"
#include "lab_crusher.h"
#include "soul_merger_control_panel.h"
#include "soul_merger.h"
#include "lava_door.h"
#include "flame_pillar.h"
#include "extend_o_grab_button.h"
#include "puzzle_piece.h"
#include "jigsaw_puzzle.h"
#include "jigsaw_puzzle_display.h"
#include "magnet.h"
#include "attractor.h"
#include "cell_bars.h"
#include "item_remover.h"
#include "safe.h"
#include "snow_pile.h"
#include "spore_crop.h"
#include "spore.h"
#include "vines.h"
#include "slime_potion_pool.h"
#include "monster_skull.h"
#include "apple_tree.h"
#include "apple.h"
#include "one_way_door.h"
#include "colour_pot.h"
#include "colour_display.h"
#include "phase_door.h"
#include "mastermind_display.h"
#include "mastermind.h"
#include "memory_match_display.h"
#include "memory_match.h"
#include "drawbridge_pulley.h"
#include "drawbridge_anchor.h"
#include "safe_combination.h"
#include "grimlore_artifact.h"
#include "hidden_passage_wall.h"
#include "poison_meat.h"
#include "train_track.h"
#include "train.h"
#include "portable_save.h"
#include "bell_rope.h"
#include "music_sheet_display.h"
#include "chaos_chain_base.h"

extern Entity *self, player;

static Constructor items[] = {
{"item/chicken_feed_bag", &addChickenFeedBag},
{"item/chicken_trap", &addChickenTrap},
{"item/coal_pile", &addCoalPile},
{"misc/small_tree", &addShrub},
{"item/heart_container", &addHeartContainer},
{"item/stalactite", &addStalactite},
{"item/bomb", &addBomb},
{"item/bomb_pile", &addBombPile},
{"item/health_potion", &addHealthPotion},
{"item/slime_potion", &addSlimePotion},
{"item/invisibility_potion", &addInvisibilityPotion},
{"item/arrow_target", &addArrowTarget},
{"item/power_generator", &addPowerGenerator},
{"item/grabber", &addGrabber},
{"item/symbol_block", &addSymbolBlock},
{"item/force_field", &addForceField},
{"item/horizontal_force_field", &addForceField},
{"item/gazer_eye_slot", &addGazerEyeSlot},
{"item/spring", &addSpring},
{"item/number_block", &addNumberBlock},
{"item/bubble_machine", &addBubbleMachine},
{"item/bubble", &addBubble},
{"item/high_striker", &addHighStriker},
{"item/potion_dispenser", &addPotionDispenser},
{"item/tesla_charger", &addTeslaCharger},
{"item/tesla_pack_empty", &addTeslaPack},
{"item/tesla_pack_full", &addTeslaPack},
{"item/containment_unit", &addContainmentUnit},
{"item/containment_unit_controls", &addContainmentUnitControls},
{"item/ice_spray", &addIceSpray},
{"item/blender", &addBlendingMachine},
{"item/spike_ball", &addSpikeBall},
{"item/code_door", &addCodeDoor},
{"item/code_display", &addCodeDisplay},
{"item/skull", &addSkull},
{"item/glass_wall", &addGlassWall},
{"item/robot", &addRobot},
{"item/tuning_fork", &addTuningFork},
{"item/pedestal", &addPedestal},
{"item/cauldron", &addCauldron},
{"item/soul_bottle", &addSoulBottle},
{"item/instruction_machine", &addInstructionMachine},
{"item/moveable_lift", &addMoveableLift},
{"item/mine_cart", &addMineCart},
{"item/rock_container", &addRockContainer},
{"item/scale", &addScale},
{"item/lava_geyzer", &addLavaGeyzer},
{"item/crusher", &addCrusher},
{"item/repellent", &addRepellent},
{"item/cloud_geyzer", &addCloudGeyzer},
{"item/slime_container", &addSlimeContainer},
{"item/final_power_generator", &addFinalPowerGenerator},
{"item/mirror", &addMirror},
{"item/upside_down_mirror", &addMirror},
{"item/light_source", &addLightSource},
{"item/light_target", &addLightTarget},
{"item/balloon", &addBalloon},
{"item/water_purifier", &addWaterPurifier},
{"item/extend_o_grab", &addExtendOGrab},
{"item/sliding_puzzle", &addSlidingPuzzle},
{"item/sliding_puzzle_display", &addSlidingPuzzleDisplay},
{"item/large_sliding_puzzle_display", &addSlidingPuzzleDisplay},
{"item/tread_mill", &addTreadMill},
{"item/power_meter", &addPowerMeter},
{"item/shield_pillar", &addShieldPillar},
{"item/crossbow", &addCrossbow},
{"item/crossbow_bolt", &addCrossbowBolt},
{"item/catapult", &addCatapult},
{"item/light_pillar", &addLightPillar},
{"item/exploding_gazer_eye", &addExplodingGazerEye},
{"item/exploding_gazer_eye_dud", &addExplodingGazerEyeDud},
{"item/lab_crusher", &addLabCrusher},
{"item/soul_merger_control_panel", &addSoulMergerControlPanel},
{"item/soul_merger", &addSoulMerger},
{"item/lava_door", &addLavaDoor},
{"item/flame_pillar", &addFlamePillar},
{"item/extend_o_grab_button", &addExtendOGrabButton},
{"item/puzzle_piece", &addPuzzlePiece},
{"item/jigsaw_puzzle", &addJigsawPuzzle},
{"item/jigsaw_puzzle_display", &addJigsawPuzzleDisplay},
{"item/magnet", &addMagnet},
{"item/attractor", &addAttractor},
{"item/horizontal_glass_wall", &addGlassWall},
{"item/cell_bars", &addCellBars},
{"item/item_remover", &addItemRemover},
{"item/safe", &addSafe},
{"item/snow_pile", &addSnowPile},
{"item/flaming_arrow_potion", &addFlamingArrowPotion},
{"item/spore_crop", &addSporeCrop},
{"item/spores", &addSpore},
{"item/vines", &addVines},
{"item/slime_potion_pool", &addSlimePotionPool},
{"item/monster_skull", &addMonsterSkull},
{"item/apple_tree", &addAppleTree},
{"item/apple", &addApple},
{"item/one_way_door", &addOneWayDoor},
{"item/one_way_door_up", &addOneWayDoor},
{"item/one_way_door_down", &addOneWayDoor},
{"item/colour_pot", &addColourPot},
{"item/colour_display", &addColourDisplay},
{"item/phase_door", &addPhaseDoor},
{"item/mastermind_display", &addMastermindDisplay},
{"item/mastermind", &addMastermind},
{"item/mastermind_peg", &addMastermindPeg},
{"item/mastermind_score", &addMastermindScore},
{"item/memory_match_display", &addMemoryMatchDisplay},
{"item/memory_match", &addMemoryMatch},
{"item/memory_match_icon", &addMemoryMatchIcon},
{"item/drawbridge_pulley", &addDrawbridgePulley},
{"item/drawbridge_anchor", &addDrawbridgeAnchor},
{"item/safe_combination", &addSafeCombination},
{"item/protection_artifact", &addProtectionArtifact},
{"item/reflection_artifact", &addReflectionArtifact},
{"item/bind_artifact", &addBindArtifact},
{"item/hidden_passage_wall", &addHiddenPassageWall},
{"item/poison_meat", &addPoisonMeat},
{"item/train_track", &addTrainTrack},
{"item/train", &addTrain},
{"item/portable_save", &addPortableSave},
{"item/bell_rope", &addBellRope},
{"item/music_sheet_display", &addMusicSheetDisplay},
{"item/chaos_chain_base", &addChaosChainBase}
};

static int length = sizeof(items) / sizeof(Constructor);

Entity *addKeyItem(char *name, int x, int y)
{
	int i;
	Entity *e;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(items[i].name, name) == 0)
		{
			e = items[i].construct(x, y, name);

			if (e->fallout == NULL)
			{
				e->fallout = &itemFallout;
			}

			return e;
		}
	}

	showErrorAndExit("Could not find key item %s", name);

	return NULL;
}

void keyItemTouch(Entity *other)
{
	if (!(self->flags & INVULNERABLE) && other->type == PLAYER && other->health > 0)
	{
		if (self->mental != -1 && strcmpignorecase(self->name, "item/health_potion") == 0)
		{
			increaseSecretsFound();
		}

		addToInventory(self);
	}
}

void keyItemFallout()
{
	self->dirX = 0;

	self->thinkTime = 120;

	self->action = &keyItemRespawn;

	self->flags &= ~FLY;
}

void keyItemRespawn()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->thinkTime <= 0)
	{
		self->x = player.x + (player.w - self->w) / 2;
		self->y = player.y + player.h - self->h;

		self->dirY = ITEM_JUMP_HEIGHT;

		setCustomAction(self, &invulnerable, 60, 0, 0);

		self->action = &doNothing;

		self->touch = &keyItemTouch;

		self->environment = AIR;
	}
}
