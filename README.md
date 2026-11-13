# Pokémon: Ash Horizon

A small, original, GBA-inspired 2D Pokémon-style adventure prototype written in C++ with SDL3.

> This is a fan-made, non-commercial learning project. Pokémon and related names/characters are trademarks of their respective owners. No official Pokémon assets are included.

## Current prototype

- GBA-like 240×160 logical resolution, scaled for a desktop window
- Tile-based grassy route with trees, paths, and water
- Keyboard movement and collision
- New-game trainer setup with a typed name (up to 12 characters) and male/female selection, followed by starter selection (Bulbasaur, Charmander, or Squirtle)
- On-screen wallet display for PokeDollars, HC, and Shards; the currencies are saved but earning and spending rewards are not connected yet
- Random encounters while walking through tall grass
- Turn-based wild battles with species-based HP and stats, type effectiveness, STAB, critical hits, friendship-based attack refusal, capture attempts, and experience rewards
- Authored four-slot starter move sets for Bulbasaur, Charmander, and Squirtle, with a small move catalogue and individual PP values stored per Pokémon
- Six-slot Pokémon party with starter and wild Pokémon, individual HP, experience/level progression, active-member switching, and battle HP linked to the active Pokémon
- One persistent save slot stores the trainer profile, gender selection, PokeDollars, HC, Shards, party, HP, friendship, experience, move slots and PP, player position, inventory, Arceus Ball timer state, and a randomized Single/Double preference; save versions 1–3 remain compatible
- Private Windows development build has a native top-bar **Cheats** menu with a separate code-entry window; `samfr2x` grants 999 of every currently defined item and makes the Arceus Ball timer unlimited in the saved game (the full capture mechanic is not implemented yet)
- Bag menu supports Potion, Super Potion, and Revive use; capture and item rules have standalone unit tests
- Standalone Pokémon stat and damage calculation core (IVs, EVs, nature, stat stages, STAB, type multiplier, random damage factor, and critical-hit modifier)
- Automated battle-stat, Pokédex, party, battle-format, and save/inventory unit tests run in Windows CI
- Lazy-loaded Pokémon sprite bank with SDL3_image support for common image formats (PNG, BMP, JPEG, GIF, WebP, TGA, TIFF, QOI, and other formats enabled by the SDL3_image build)

## Controls

- `W A S D`: move around the world
- In battle, `W` / `S`: move between move-menu rows; `A` / `D`: move between columns
- `E`: confirm a move, confirm a starter, use a Bag item, switch a selected party member, or continue after battle
- `T`: open the party from the world, close the Bag/party screen, or switch to the next party Pokémon during battle (uses the opponent turn)
- `I`: open the Bag
- `F`: throw the selected capture ball during a wild battle
- `Z`: cycle Poké Ball, Great Ball, Ultra Ball, Master Ball, and Arceus Ball
- `G`: attempt to escape a wild battle
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
- `src/battle_format.hpp` — per-save randomized Single/Double preference and party-size constraints
- `src/player_profile.hpp` — trainer name, gender, and the three-currency wallet model
- `src/inventory.hpp` — item quantities and Arceus Ball timer state
- `src/save_game.hpp` — persistent save serialization for profile, currencies, battle rules, party, inventory, position, and timer state
- `src/private_cheat_menu.hpp` — private-build native desktop Cheats menu and separate code-entry window
- `tests/pokemon_party_test.cpp` — party storage, HP, healing, and switching tests
- `tests/save_game_test.cpp` — save/load, profile, wallet, inventory, and Arceus Ball timer tests
- `tests/pokemon_data_test.cpp` — Pokédex data validation and loader tests
- `tests/battle_stats_test.cpp` — unit tests for the stat calculation core
- `build_32bit.bat` — Windows x86 build script
- `src/pokemon_sprite_bank.hpp` — lazy sprite loading with common image-format support
- `assets/README.md` — notes for adding original pixel-art assets

## Next milestones

1. Add player appearance and hair-style choices to the trainer setup.
2. Add Shards and HC as earnable/spendable progression resources, plus ordinary PokeDollar rewards and shops.
3. Build the Astralis Region town and route-transition system from the approved map, then expand to Silverpeak Town and its NPCs.
4. Add gym missions, badge-gated field moves, and the Hagemony story progression.
5. Populate the Pokédex data pack with verified Gen 1–9 species, forms, and evolution records.
6. Add original player and Pokémon sprites and replace the remaining rectangle placeholders.

This is an early playable gameplay slice, not a complete Pokémon game. Starter selection, basic wild capture, item healing/revival, experience progression, and save persistence are connected. The trainer name/gender setup and wallet display are connected to the game; appearance customization and currency earning/spending systems are still pending. Trainer battles, the full Pokédex, move-learning progression, status conditions, evolution, and the complete Double Battle engine are also unfinished.
