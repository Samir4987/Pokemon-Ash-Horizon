# Ash Horizon asset pipeline

The game uses a lazy-loading sprite bank. It loads only the images currently requested,
which helps keep memory use low on the target 32-bit / 2 GB PC.

## Pokémon sprite paths

Use one folder per National Pokédex number, zero-padded to four digits:

```
assets/pokemon/0001/normal/front.bmp
assets/pokemon/0001/normal/back.bmp
assets/pokemon/0001/normal/overworld.bmp
assets/pokemon/0025/normal/front.bmp
assets/pokemon/0025/normal/back.bmp
assets/pokemon/0025/normal/overworld.bmp
assets/pokemon/0025/shiny/front.bmp
assets/pokemon/0025/shiny/back.bmp
assets/pokemon/0025/shiny/overworld.bmp
assets/pokemon/0025/alolan/front.bmp
```

- Supported National Dex range in the loader: 1–1025 (Generations 1–9).
- Form names are folder names such as `normal`, `shiny`, `alolan`, or a project-defined regional form.
- Views are `front`, `back`, and `overworld`.
- Files are BMP for the initial SDL3-only pipeline; convert approved source art to BMP before adding it.
- Keep source art at native pixel-art resolution; do not upscale it in the asset files.
- Missing sprites are safely reported as unavailable so the game can use a temporary fallback.
- Textures are loaded on demand and cached; the bank releases them on shutdown.

## Asset rights and provenance

This repository does not bundle ripped official Pokémon artwork. Add original artwork,
user-created artwork, or assets whose license explicitly permits the intended use. Keep
a source and license note alongside any third-party pack. The loader supports the complete
Gen 1–9 index, but the sprite files themselves must be supplied and checked in separately.

## Other assets

- `assets/sprites/` — player and NPC sprites
- `assets/tilesets/` — route, town, and interior tiles
- `assets/audio/` — music and sound effects
