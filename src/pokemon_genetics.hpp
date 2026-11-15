/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#pragma once

#include "battle_stats.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ash_horizon {

// Stable IDs for the standard 25 Pokémon natures. The five neutral natures
// are represented explicitly so save files can store a compact numeric ID.
enum class NatureId : std::uint8_t {
    Hardy, Lonely, Brave, Adamant, Naughty,
    Bold, Docile, Relaxed, Impish, Lax,
    Timid, Hasty, Serious, Jolly, Naive,
    Modest, Mild, Quiet, Bashful, Rash,
    Calm, Gentle, Sassy, Careful, Quirky
};

inline constexpr std::array<Nature, 25> NATURE_TABLE{{
    {Stat::None, Stat::None}, {Stat::Attack, Stat::Defense},
    {Stat::Attack, Stat::Speed}, {Stat::Attack, Stat::SpecialAttack},
    {Stat::Attack, Stat::SpecialDefense},
    {Stat::Defense, Stat::Attack}, {Stat::None, Stat::None},
    {Stat::Defense, Stat::Speed}, {Stat::Defense, Stat::SpecialAttack},
    {Stat::Defense, Stat::SpecialDefense},
    {Stat::Speed, Stat::Attack}, {Stat::Speed, Stat::Defense},
    {Stat::None, Stat::None}, {Stat::Speed, Stat::SpecialAttack},
    {Stat::Speed, Stat::SpecialDefense},
    {Stat::SpecialAttack, Stat::Attack}, {Stat::SpecialAttack, Stat::Defense},
    {Stat::SpecialAttack, Stat::Speed}, {Stat::None, Stat::None},
    {Stat::SpecialAttack, Stat::SpecialDefense},
    {Stat::SpecialDefense, Stat::Attack}, {Stat::SpecialDefense, Stat::Defense},
    {Stat::SpecialDefense, Stat::Speed}, {Stat::SpecialDefense, Stat::SpecialAttack},
    {Stat::None, Stat::None}
}};

inline constexpr std::size_t NATURE_COUNT = NATURE_TABLE.size();
inline constexpr int MAX_TOTAL_EVS = 510;
inline constexpr int MAX_SINGLE_STAT_EVS = 252;
inline constexpr int MAX_IV_VALUE = 31;

inline Nature natureFromId(std::uint8_t id) {
    return NATURE_TABLE[id < NATURE_COUNT ? id : 0];
}

inline bool isValidIvSpread(const std::array<int, 6>& ivs) {
    for (int value : ivs)
        if (value < 0 || value > MAX_IV_VALUE) return false;
    return true;
}

inline bool isValidEvSpread(const std::array<int, 6>& evs) {
    int total = 0;
    for (int value : evs) {
        if (value < 0 || value > MAX_SINGLE_STAT_EVS) return false;
        total += value;
    }
    return total <= MAX_TOTAL_EVS;
}

} // namespace ash_horizon
