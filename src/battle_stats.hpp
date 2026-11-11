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

#include <algorithm>
#include <array>
#include <cstdint>

namespace ash_horizon {

enum class Stat : std::uint8_t { Attack, Defense, SpecialAttack, SpecialDefense, Speed, None };

struct Nature {
    Stat raised = Stat::None;
    Stat lowered = Stat::None;
};

struct StatInputs {
    int level = 1;
    int baseHp = 1;
    int baseAttack = 1;
    int baseDefense = 1;
    int baseSpecialAttack = 1;
    int baseSpecialDefense = 1;
    int baseSpeed = 1;
    std::array<int, 6> ivs{{0, 0, 0, 0, 0, 0}};
    std::array<int, 6> evs{{0, 0, 0, 0, 0, 0}};
    Nature nature{};
};

struct BattleStats {
    int hp = 1;
    int attack = 1;
    int defense = 1;
    int specialAttack = 1;
    int specialDefense = 1;
    int speed = 1;
};

inline int clampIv(int value) { return std::clamp(value, 0, 31); }
inline int clampEv(int value) { return std::clamp(value, 0, 252); }
inline int clampLevel(int value) { return std::clamp(value, 1, 100); }

inline int calculateHpStat(int base, int iv, int ev, int level) {
    const int scaled = ((2 * std::max(1, base) + clampIv(iv) + clampEv(ev) / 4) * clampLevel(level)) / 100;
    return scaled + clampLevel(level) + 10;
}

inline int calculateOtherStat(int base, int iv, int ev, int level,
                              Stat stat, Nature nature) {
    const int safeLevel = clampLevel(level);
    const int scaled = ((2 * std::max(1, base) + clampIv(iv) + clampEv(ev) / 4) * safeLevel) / 100;
    int value = scaled + 5;
    if (nature.raised == stat && nature.lowered != stat) value = value * 110 / 100;
    else if (nature.lowered == stat && nature.raised != stat) value = value * 90 / 100;
    return std::max(1, value);
}

inline BattleStats calculateBattleStats(const StatInputs& in) {
    return {
        calculateHpStat(in.baseHp, in.ivs[0], in.evs[0], in.level),
        calculateOtherStat(in.baseAttack, in.ivs[1], in.evs[1], in.level, Stat::Attack, in.nature),
        calculateOtherStat(in.baseDefense, in.ivs[2], in.evs[2], in.level, Stat::Defense, in.nature),
        calculateOtherStat(in.baseSpecialAttack, in.ivs[3], in.evs[3], in.level, Stat::SpecialAttack, in.nature),
        calculateOtherStat(in.baseSpecialDefense, in.ivs[4], in.evs[4], in.level, Stat::SpecialDefense, in.nature),
        calculateOtherStat(in.baseSpeed, in.ivs[5], in.evs[5], in.level, Stat::Speed, in.nature)
    };
}

// Generation-style stat-stage multipliers: -6..+6.
inline int applyStatStage(int stat, int stage) {
    const int safeStat = std::max(1, stat);
    const int s = std::clamp(stage, -6, 6);
    if (s >= 0) return safeStat * (2 + s) / 2;
    return safeStat * 2 / (2 - s);
}

// Damage core. The caller supplies the already-resolved type multiplier and
// random factor (0.85..1.00), making battles reproducible in tests.
inline int calculateDamage(int level, int power, int attack, int defense,
                          bool sameTypeAttackBonus, float typeMultiplier,
                          float randomFactor = 1.0f, bool critical = false) {
    if (power <= 0 || typeMultiplier <= 0.0f) return 0;
    const int safeLevel = clampLevel(level);
    const int safeAttack = std::max(1, attack);
    const int safeDefense = std::max(1, defense);
    const std::int64_t levelTerm = (2LL * safeLevel) / 5 + 2;
    std::int64_t base = (((levelTerm * power * safeAttack) / safeDefense) / 50) + 2;
    if (critical) base = (base * 3) / 2;
    const float stab = sameTypeAttackBonus ? 1.5f : 1.0f;
    const float boundedRandom = std::clamp(randomFactor, 0.0f, 1.0f);
    const float result = static_cast<float>(base) * stab * typeMultiplier * boundedRandom;
    return std::max(1, static_cast<int>(result));
}

} // namespace ash_horizon
