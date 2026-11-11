# Ash Horizon sprite-sheet assets

The game loads sprite sheets lazily. Each sheet contains many equal-sized sprite cells;
the loader calculates the correct cell from the National Pokédex number. It does not
require one image file per Pokémon.

## Default Pokémon sprite sheets

Put sheets here:

```
assets/pokemon/sheets/normal_front.png
assets/pokemon/sheets/normal_back.png
assets/pokemon/sheets/normal_overworld.png
assets/pokemon/sheets/shiny_front.png
assets/pokemon/sheets/shiny_back.png
assets/pokemon/sheets/shiny_overworld.png
assets/pokemon/sheets/alolan_front.png
```

Default sheet layout:

- Each cell is 64 × 64 pixels.
- There are 16 columns.
- National Dex #1 is the first cell (top-left).
- National Dex #2 is the next cell to its right, continuing row by row.
- The loader supports National Dex numbers 1–1025.
- Missing sheets use the game's temporary fallback graphics.

If your sheet uses another cell size or column count, register it in code before the
first sprite request with `PokemonSpriteBank::configureSheet(form, view, path,
spriteWidth, spriteHeight, columns, firstDex)`.

Example:

```cpp
spriteBank.configureSheet("normal", "front",
    "assets/pokemon/sheets/all_front.png", 80, 80, 12);
```

The sheet is loaded once and cached. Individual sprite files are not needed.

## Other assets

- `assets/sprites/` — player and NPC sprite sheets
- `assets/tilesets/` — route, town, and interior tilesets
- `assets/audio/` — music and sound effects

## Asset rights and provenance

Add original artwork, user-created artwork, or assets whose license explicitly permits
the intended use. Keep a source and license note alongside third-party asset packs.
The loader supports the Gen 1–9 Pokédex index; sprite artwork must be supplied separately.
