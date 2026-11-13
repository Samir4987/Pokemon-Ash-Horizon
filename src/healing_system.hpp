/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "pokemon_party.hpp"

namespace ash_horizon {

class HealingSystem {
public:
    static bool healAtCenter(PokemonParty& party) {
        if (party.empty()) return false;
        party.healAll();
        return true;
    }

    static bool allFainted(const PokemonParty& party) {
        return !party.empty() && party.usableCount() == 0;
    }

    // Called only after a battle ends with every party member fainted.
    static bool emergencyReturn(PokemonParty& party, float& playerX, float& playerY,
                                float centerSpawnX, float centerSpawnY) {
        if (!allFainted(party)) return false;
        party.healAll();
        playerX = centerSpawnX;
        playerY = centerSpawnY;
        return true;
    }
};

} // namespace ash_horizon
