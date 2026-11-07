# Ash Horizon — Living World Design

This document records the currently agreed direction for Pokémon: Ash Horizon. It describes planned features; it does not claim they are implemented.

## Project constraints
- C++ with SDL3.
- 32-bit Windows (x86) target.
- GBA-inspired presentation and 240×160 logical resolution.
- Keep memory and CPU use modest for the target PC (2 GB RAM, Intel Celeron B815, Intel HD Graphics).
- Simulate nearby activity in detail; use simplified or paused simulation for distant areas.

## World principle
Astralis should feel like a habitat that exists independently of the player. Pokémon and NPCs should continue their routines without requiring the player to trigger every event.

## Living Pokémon ecosystem
Planned overworld behaviours:
- Species prefer suitable habitats and encounter conditions.
- Pokémon wander, rest, sleep, play, gather, and travel in groups.
- Compatible species may play or form social groups.
- Territorial species may display warnings or compete for space.
- Parent Pokémon may protect young.
- Predator/prey relationships can be represented through non-graphic chase, avoidance, and retreat behaviours.
- Frightened Pokémon may flee; curious Pokémon may investigate.
- Pokémon may seek shelter or safe locations when tired or during severe weather.
- Behaviours should be species- and habitat-aware rather than identical random animations.

Interactions should occur autonomously in the overworld. The player may observe them without having to start a mission or battle.

## Time and environment
Planned systems:
- Day/night cycle.
- Weather such as rain, snow, fog, storms, and heat.
- Time- and weather-dependent species activity.
- Weather and time can change routes, ambience, and available encounters.

## NPC life simulation
Planned systems:
- Lightweight daily routines for selected NPCs.
- Trainers may travel between settlements.
- Shops and public places can follow schedules.
- NPC dialogue may react to story events, Gym victories, and changes caused by Team Hegemony.
- Gym Leaders may appear outside their Gyms when appropriate to the story.

## Player and Pokémon relationship
- Pokémon can follow the player where appropriate.
- Pokémon may react to locations, weather, and nearby creatures.
- Friendship affects obedience in battle.
- Species can have distinct reactions and temperaments.

### Battle obedience rule
Run a random obedience check whenever the player issues an attack command:
- Friendship above 84%: 0% refusal chance.
- Friendship from 40% through 84%: 10% refusal chance.
- Friendship below 40%: 60% refusal chance.
- At exactly 40% and exactly 84%, use the middle tier.
- If the Pokémon refuses, it does not execute that selected attack for that turn.

## Existing confirmed game requirements
- GBA-style presentation throughout.
- Opening disclaimer screens, followed by the title screen:
  - POKÉMON: ASH HORIZON
  - Created by Fan_Made_Sensei
  - No subtitle.
- Custom player name entry with an on-screen keyboard.
- Pokémon nickname choice after obtaining a Pokémon.
- Pokémon from Generations 1–9 are planned.
- Mega Evolution, Z-Moves, Dynamax/Gigantamax, and Terastallization remain distinct mechanics.
- No traditional HM system: Pokémon use suitable learned moves outside battle, with Gym Badges granting permission to use field moves. Exact move-to-badge mapping remains undecided.
- Three confirmed starter evolution lines:
  - Grass: Leaftot → Fernox → Sylvarion (final form Grass/Fairy).
  - Fire: Calflare → Pyrohart → Noctiferno (final form Fire/Dark).
  - Water: Bubbleshell → Corallume → Psycoral (final form Water/Psychic).
- Twelve Gym challenges, each with a unique pre-Gym mission.
- Three Hegemony statue-robots appear after 4, 8, and 12 badges, at levels 48, 72, and 100 respectively.
- One Master Ball is located in Team Hegemony Main Office.
- Astralis geography and the user's supplied map should be preserved.

## Implementation strategy
Build and test in small layers:
1. Stable core loop, input, collision, and map rendering.
2. Data-driven tile maps and interactive objects.
3. A small number of visible overworld Pokémon with deterministic, lightweight state machines.
4. Habitat rules and autonomous interactions between nearby Pokémon.
5. Time-of-day and weather hooks.
6. NPC schedules and event-reactive dialogue.
7. Persistent party, friendship, catching, and battle systems.
8. Expand the Astralis story, Gym missions, and regional content.

Avoid simulating every Pokémon in the whole region every frame. Update nearby entities at a modest tick rate, use simple distance checks, and pause or summarize activity outside the active area.

## Current prototype status
The repository currently contains an early SDL3 prototype with a placeholder route, player movement, collision, random grass encounters, and a basic battle screen. The living-world systems and the other requirements above are design targets, not implemented features yet.