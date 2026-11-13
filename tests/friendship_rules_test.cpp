/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "../src/friendship_rules.hpp"
#include <cassert>

int main() {
    using ash_horizon::attackRefusalChance;

    // Low-friendship range: 60%.
    assert(attackRefusalChance(0) == 60);
    assert(attackRefusalChance(1) == 60);
    assert(attackRefusalChance(39) == 60);

    // The normal range includes both boundaries: 10%.
    assert(attackRefusalChance(40) == 10);
    assert(attackRefusalChance(41) == 10);
    assert(attackRefusalChance(84) == 10);

    // Above 84 friendship, refusal is impossible.
    assert(attackRefusalChance(85) == 0);
    assert(attackRefusalChance(99) == 0);
    assert(attackRefusalChance(100) == 0);

    // Defensive clamping for invalid input.
    assert(attackRefusalChance(-20) == 60);
    assert(attackRefusalChance(120) == 0);
    return 0;
}
