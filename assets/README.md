# Ash Horizon graphics

Graphics are loaded lazily through SDL3_image. Keep imported artwork in this local
folder; the repository ignores the art directories so third-party artwork is not
accidentally redistributed.

## Import the supplied archives

From the repository root, run:

```bat
python tools\import_graphics.py "Generation 9 Pack v3.3.8(1).zip" "Downloads(1).zip"
```

The importer creates:

- `assets/pokemon/battle/front/` — individual front battle sprites.
- `assets/pokemon/battle/back/` — individual back battle sprites.
- `assets/pokemon/followers/` and `followers_shiny/` — 4x4 directional follower sheets.
- `assets/pokemon/overworld/{down,up,left,right}/` — 32x32 directional sprites.
- `assets/battle/balls/` — ball and ball-opening artwork from the inspected pack.

The game uses the species name (uppercased) to locate individual battle and follower
sprites. The overworld archive uses numeric National Dex filenames and currently
contains IDs 1–493. Missing art always falls back to the simple built-in graphics.

## Trainer artwork and move animations

The inspected archives did not contain a usable trainer sprite set or a complete
library of move-specific battle animations. Optional player images can be placed at:

```
assets/trainers/player_down.png
assets/trainers/player_up.png
assets/trainers/player_left.png
assets/trainers/player_right.png
```

The sprite bank also supports horizontal animation strips through
`drawAnimationStrip`; each frame must have the same dimensions. No move animation
files are fabricated by the importer.

## Sprite-sheet fallback

The original 64x64, 16-column Pokémon sheet loader remains available for compatible
sheets in `assets/pokemon/sheets/`. It is not required when using the individual
species-named battle sprites imported above.

## Rights and provenance

Only use assets whose source terms permit your intended use and distribution. This
project does not commit the imported artwork. Keep source/license information with
any asset pack you choose to use.
