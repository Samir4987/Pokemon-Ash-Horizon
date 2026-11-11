/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#include "../src/battle_stats.hpp"
#include <cassert>

int main() {
    using namespace ash_horizon;

    StatInputs inputs;
    inputs.level = 50;
    inputs.baseHp = 100;
    inputs.baseAttack = 100;
    inputs.baseDefense = 100;
    inputs.baseSpecialAttack = 100;
    inputs.baseSpecialDefense = 100;
    inputs.baseSpeed = 100;
    inputs.ivs = {{31, 31, 31, 31, 31, 31}};
    // A legal 504-EV spread: 252 Attack, 252 Speed.
    inputs.evs = {{0, 252, 0, 0, 0, 252}};

    const BattleStats stats = calculateBattleStats(inputs);
    assert(stats.hp == 175);
    assert(stats.attack == 152);
    assert(stats.defense == 120);
    assert(stats.specialAttack == 120);
    assert(stats.specialDefense == 120);
    assert(stats.speed == 152);

    inputs.nature = {Stat::Attack, Stat::Speed};
    const BattleStats natureStats = calculateBattleStats(inputs);
    assert(natureStats.attack == 167);
    assert(natureStats.speed == 136);

    assert(applyStatStage(100, 2) == 200);
    assert(applyStatStage(100, -2) == 50);
    assert(applyStatStage(100, 99) == 400);
    assert(applyStatStage(100, -99) == 25);

    assert(calculateDamage(50, 50, 100, 100, true, 2.0f, 1.0f) > 0);
    assert(calculateDamage(50, 50, 100, 100, false, 0.0f, 1.0f) == 0);
    assert(calculateDamage(50, 50, 100, 100, false, 1.0f, 1.0f) <
           calculateDamage(50, 50, 100, 100, true, 1.0f, 1.0f));
    return 0;
}
