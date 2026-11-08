# Pokémon: Ash Horizon

A small, original, GBA-inspired 2D Pokémon-style adventure prototype written in C++ with SDL3.

> This is a fan-made, non-commercial learning project. Pokémon and related names/characters are trademarks of their respective owners. No official Pokémon assets are included.

## Current prototype

- GBA-like 240×160 logical resolution, scaled for a desktop window
- Tile-based grassy route with trees, paths, and water
- Keyboard movement and collision
- Random encounters while walking through tall grass
- A simple turn-based battle prototype with Attack and Run
- Standalone Pokémon stat and damage calculation core (IVs, EVs, nature, stat stages, STAB, type multiplier, random damage factor, and critical-hit modifier)
- Automated battle-stat unit tests run in Windows CI
- No external image or audio files are required yet

## Controls

- Arrow keys / WASD: move
- Enter / Space: confirm or attack
- Escape: quit

## Build on 32-bit Windows

This project targets 32-bit Windows (x86) and SDL3.

1. Install an **i686 MinGW-w64** C++ compiler.
2. Download the SDL3 **Win32 / x86 development package**.
3. Edit `build_32bit.bat` and set `SDL_DIR` to the folder containing SDL3's `include` and `lib` directories.
4. Run `build_32bit.bat`.
5. Copy `SDL3.dll` from SDL3's `bin` folder beside `PokemonAshHorizon.exe`, then run the EXE.

The script uses `g++ -m32`; the compiler itself must have 32-bit MinGW-w64 support. If your SDL package has a different folder layout, adjust the paths in the script.

## Project layout

- `src/main.cpp` — game loop, map, movement, encounters, and battle
- `src/battle_system.hpp` — battle flow, moves, friendship refusal, and attack effects
- `src/battle_stats.hpp` — standalone Pokémon stat and damage calculation core
- `tests/battle_stats_test.cpp` — unit tests for the stat calculation core
- `build_32bit.bat` — Windows x86 build script
- `assets/README.md` — notes for adding original pixel-art assets

## Next milestones

1. Connect calculated Pokémon stats to the battle engine and replace placeholder combat values.
2. Add the complete 18-type effectiveness chart and dual-type handling.
3. Add status conditions, critical-hit rolls, and move effects.
4. Add original player and Pokémon sprites.
5. Add dialogue boxes, a title screen, party management, a Pokédex, maps, NPCs, and save data.

This is an early playable prototype, not a complete Pokémon game.
