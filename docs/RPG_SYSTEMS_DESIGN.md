# Ash Horizon — RPG, Quest, and Economy Design

This document records confirmed design decisions. Items marked **Open** are not yet specified and must not be silently invented during implementation.

## RPG direction

Ash Horizon is a Pokémon RPG with a living world, exploration, battles, quests, NPC routines, cooking, and meaningful progression. Main-story progression remains primary; optional systems must not block the main story.

## Quest tracker

Quest categories:
- Main quests
- Side quests
- Pokémon-related quests
- Restaurant/cooking quests
- Exploration quests

The player may have multiple active quests but chooses one tracked quest for the gameplay HUD. The tracker supports objective checklists, map markers, progress updates, rewards, and completed/failed history.

## Dynamic NPC quests

NPCs can offer randomly generated quests. Quest generation must respect the NPC's location, role, personality, and circumstances. Examples include delivery, finding a missing Pokémon, gathering ingredients, trainer battles, rescue, investigation, escort, and discovery.

Quest rewards may include Poké Dollars, Horizon Coins, items, friendship, or experience. Repeated quest templates should use cooldowns and variety controls to avoid spam.

**Confirmed Horizon Coin reward:** random NPC quests can award 1–6 HC. Difficulty should influence the reward range; exact probability weights are **Open**.

## Currencies

### Poké Dollars (P$)
Everyday currency for normal shops and services.

### Horizon Coins (HC)
Special quest/reward currency.
- Fixed exchange rate: **1 HC = 1,400 P$**.
- Horizon Coins can be exchanged for Poké Dollars at special NPC exchange counters.
- NPC quests can award 1–6 HC.
- Horizon Coins are used by the Horizon Shop.

### Genesis Shards (GS)
Rare currency available during the main story, not restricted to postgame.
- Sources include story progression and rare discoveries; exact reward tables are **Open**.
- Genesis Shards can be bought for **999,999 P$ each**.
- Genesis Shards can be sold for **450,000 P$ each**.
- Buying and selling use these fixed prices.

## Horizon Shop

The Horizon Shop is an NPC-operated random-reward shop inspired by the old Blox Fruits NPC gacha interaction:
- No flashy summon animation, wheel, or cinematic.
- The player talks to the NPC, pays, and receives the result immediately.
- Each purchase gives a random quantity of **1–8 items**.
- The shop is intentionally shady and may produce disappointing rewards; the exact probability display/knowledge is intentionally not shown to the player.
- Genesis Shards can appear as an exceptionally rare reward: **0.00001% per roll**.
- Daily limit: **60 rolls per in-game day**.
- Maximum Genesis Shards from this gacha: **1 per in-game day**.
- Genesis Shards remain obtainable through story progression and other non-gacha sources.

**Open:** Horizon Shop price per roll, full reward pool, item quantity distribution, and odds for non-Genesis rewards.

## Genesis Shard exclusive item

The Genesis Shard shop has only one purchasable item: the **Arceus Ball**.
- Seller: Poké Ball Company Owner.
- Price: **10 Genesis Shards**.
- Purchase limit: **1 per in-game year**.
- The Arceus Ball can target only Arceus.
- The story allows the player to capture **only one Arceus**.

### Arceus capture rules
- Any other Poké Ball used on Arceus, including a Master Ball, shatters.
- The Arceus Ball has an actual **85% catch chance**.
- The 85% probability is not disclosed to the player because the item is an experimental prototype.
- The ball is intended to be a risky, story-significant capture attempt.

**Open:** What happens after a failed Arceus Ball attempt, and whether/how another ball can be obtained after the annual purchase limit.

## Restaurant and cooking

Ash Horizon includes cooking and restaurant activities, with vegetarian and non-vegetarian dishes, recipe discovery, ingredients, NPC customers, and optional restaurant progression. Food sourcing and exact Pokémon-world lore rules remain **Open** and should be settled before implementation.

## Current battle-engine status

The current C++ battle engine is an early prototype:
- Four placeholder moves: Tackle, Ember, Water Gun, Thunder Shock.
- Basic turn phases, HP, simple effectiveness checks, and particle effects.
- The type chart is incomplete.
- The damage formula is a placeholder and is not yet Generation 9 accurate.
- IVs, EVs, critical hits, major status conditions, stat stages, friendship-based refusal, abilities, and held items are design requirements but are not yet implemented.
- The friendship-based refusal rule is:
  - 85–100% friendship: 0% refusal.
  - 40–84% friendship: 10% refusal.
  - 0–39% friendship: 60% refusal.

Do not describe the prototype as a complete Pokémon battle system. Implement and test mechanics in small, verifiable steps.
