/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "../src/pokemon_genetics.hpp"
#include <array>
#include <cassert>

int main() {
    using namespace ash_horizon;

    static_assert(NATURE_COUNT == 25);
    assert(natureFromId(static_cast<std::uint8_t>(NatureId::Adamant)).raised == Stat::Attack);
    assert(natureFromId(static_cast<std::uint8_t>(NatureId::Adamant)).lowered == Stat::SpecialAttack);
    assert(natureFromId(static_cast<std::uint8_t>(NatureId::Timid)).raised == Stat::Speed);
    assert(natureFromId(static_cast<std::uint8_t>(NatureId::Timid)).lowered == Stat::Attack);
    assert(natureFromId(static_cast<std::uint8_t>(NatureId::Hardy)).raised == Stat::None);
    assert(natureFromId(255).raised == Stat::None);

    assert(isValidIvSpread({{0, 31, 15, 1, 30, 31}}));
    assert(!isValidIvSpread({{0, 32, 15, 1, 30, 31}}));
    assert(!isValidIvSpread({{-1, 0, 0, 0, 0, 0}}));

    assert(isValidEvSpread({{252, 252, 6, 0, 0, 0}}));
    assert(isValidEvSpread({{0, 0, 0, 0, 0, 0}}));
    assert(!isValidEvSpread({{252, 252, 7, 0, 0, 0}}));
    assert(!isValidEvSpread({{253, 0, 0, 0, 0, 0}}));
    assert(!isValidEvSpread({{-1, 0, 0, 0, 0, 0}}));
    return 0;
}
