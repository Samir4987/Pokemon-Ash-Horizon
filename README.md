# Pokémon: Ash Horizon

A small, original, GBA-inspired 2D Pokémon-style adventure prototype written in C++ with SDL3.

> This is a fan-made, non-commercial learning project. Pokémon and related names/characters are trademarks of their respective owners. No official Pokémon assets are included.

## Current prototype

- GBA-like 240×160 logical resolution, scaled for a desktop window
- Tile-based grassy route with trees, paths, and water
- Keyboard movement and collision
- Random encounters while walking through tall grass
- A turn-based battle prototype with a four-move menu, HP bars, battle messages, type effectiveness, and friendship-based attack refusal
- Six-slot Pokémon party with species names, levels, individual HP, active-member switching, and battle HP linked to the active Pokémon
- Standalone Pokémon stat and damage calculation core (IVs, EVs, nature, stat stages, STAB, type multiplier, random damage factor, and critical-hit modifier)
- Automated battle-stat unit tests run in Windows CI
- Lazy-loaded Pokémon sprite bank with SDL3_image support for common image formats (PNG, BMP, JPEG, GIF, WebP, TGA, TIFF, QOI, and other formats enabled by the SDL3_image build)

## Controls

- `W A S D`: move up, left, down, and right
- In battle, `W` / `S`: move between menu rows; `A` / `D`: move between columns
- `E`: confirm a battle move, switch the selected party member, or continue after a battle
- `T`: open or close the Pokémon party screen
- The operating-system window close button closes the game

The approved gameplay key whitelist is exactly `W E T I A S D F G H J K Z X C V B M`. Do not bind any other keyboard keys.

## Build on 32-bit Windows

This project targets 32-bit Windows (x86) and SDL3.

1. Install an **i686 MinGW-w64** C++ compiler.
2. Download the SDL3 **Win32 / x86 development package** and the matching SDL3_image **32-bit MinGW development package**.
3. Edit `build_32bit.bat` and set `SDL_DIR` and `SDL_IMAGE_DIR` to the folders containing each library's `include`, `lib`, and `bin` directories.
4. Run `build_32bit.bat`.
5. Keep both `SDL3.dll` and `SDL3_image.dll` beside `PokemonAshHorizon.exe`, then run the EXE.

The script uses `g++ -m32`; the compiler itself must have 32-bit MinGW-w64 support. If your SDL package has a different folder layout, adjust the paths in the script.

## Project layout

- `src/main.cpp` — game loop, map, movement, encounters, and battle
- `src/battle_system.hpp` — battle flow, moves, friendship refusal, and attack effects
- `src/battle_stats.hpp` — standalone Pokémon stat and damage calculation core
- `src/pokemon_data.hpp` — lightweight species, form, evolution, and local TSV data-loading foundation
- `src/pokemon_party.hpp` — six-slot party, Pokémon HP, levels, friendship, and switching
- `tests/pokemon_party_test.cpp` — party storage, HP, healing, and switching tests
- `tests/pokemon_data_test.cpp` — Pokédex data validation and loader tests
- `tests/battle_stats_test.cpp` — unit tests for the stat calculation core
- `build_32bit.bat` — Windows x86 build script
- `src/pokemon_sprite_bank.hpp` — lazy sprite loading with common image-format support
- `assets/README.md` — notes for adding original pixel-art assets

## Next milestones

1. Populate the Pokédex data pack with verified Gen 1–9 species, forms, and evolution records.
2. Connect calculated Pokémon stats to the battle engine and replace placeholder combat values.
3. Add status conditions, critical-hit rolls, and move effects.
4. Add original player and Pokémon sprites and replace the remaining rectangle placeholders.
5. Add dialogue boxes, a title screen, maps, NPCs, catching, and save data.

This is an early playable prototype, not a complete Pokémon game.
