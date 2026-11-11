/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#include "../src/type_chart.hpp"
#include <cassert>

int main() {
    using namespace ash_horizon;

    assert(typeEffectiveness(BattleType::Normal, BattleType::Rock) == 0.5f);
    assert(typeEffectiveness(BattleType::Fire, BattleType::Grass) == 2.0f);
    assert(typeEffectiveness(BattleType::Water, BattleType::Ground) == 2.0f);
    assert(typeEffectiveness(BattleType::Electric, BattleType::Water) == 2.0f);
    assert(typeEffectiveness(BattleType::Grass, BattleType::Rock) == 2.0f);
    assert(typeEffectiveness(BattleType::Ice, BattleType::Dragon) == 2.0f);
    assert(typeEffectiveness(BattleType::Fighting, BattleType::Normal) == 2.0f);
    assert(typeEffectiveness(BattleType::Poison, BattleType::Fairy) == 2.0f);
    assert(typeEffectiveness(BattleType::Ground, BattleType::Steel) == 2.0f);
    assert(typeEffectiveness(BattleType::Flying, BattleType::Bug) == 2.0f);
    assert(typeEffectiveness(BattleType::Psychic, BattleType::Poison) == 2.0f);
    assert(typeEffectiveness(BattleType::Bug, BattleType::Dark) == 2.0f);
    assert(typeEffectiveness(BattleType::Rock, BattleType::Flying) == 2.0f);
    assert(typeEffectiveness(BattleType::Ghost, BattleType::Psychic) == 2.0f);
    assert(typeEffectiveness(BattleType::Dragon, BattleType::Dragon) == 2.0f);
    assert(typeEffectiveness(BattleType::Dark, BattleType::Ghost) == 2.0f);
    assert(typeEffectiveness(BattleType::Steel, BattleType::Fairy) == 2.0f);
    assert(typeEffectiveness(BattleType::Fairy, BattleType::Dark) == 2.0f);

    assert(typeEffectiveness(BattleType::Normal, BattleType::Ghost) == 0.0f);
    assert(typeEffectiveness(BattleType::Electric, BattleType::Ground) == 0.0f);
    assert(typeEffectiveness(BattleType::Fighting, BattleType::Ghost) == 0.0f);
    assert(typeEffectiveness(BattleType::Poison, BattleType::Steel) == 0.0f);
    assert(typeEffectiveness(BattleType::Ground, BattleType::Flying) == 0.0f);
    assert(typeEffectiveness(BattleType::Psychic, BattleType::Dark) == 0.0f);
    assert(typeEffectiveness(BattleType::Ghost, BattleType::Normal) == 0.0f);
    assert(typeEffectiveness(BattleType::Dragon, BattleType::Fairy) == 0.0f);

    assert(typeEffectiveness(BattleType::Water, BattleType::Rock, BattleType::Ground) == 4.0f);
    assert(typeEffectiveness(BattleType::Fire, BattleType::Grass, BattleType::Steel) == 4.0f);
    assert(typeEffectiveness(BattleType::Electric, BattleType::Water, BattleType::Flying) == 4.0f);
    assert(typeEffectiveness(BattleType::Electric, BattleType::Water, BattleType::Ground) == 0.0f);
    assert(typeEffectiveness(BattleType::Fire, BattleType::Fire, BattleType::Water) == 0.25f);

    return 0;
}
