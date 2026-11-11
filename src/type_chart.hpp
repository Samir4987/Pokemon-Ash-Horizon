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
#include <cstdint>
#include <initializer_list>

namespace ash_horizon {

enum class BattleType : std::uint8_t {
    Normal, Fire, Water, Electric, Grass, Ice, Fighting, Poison, Ground,
    Flying, Psychic, Bug, Rock, Ghost, Dragon, Dark, Steel, Fairy
};

inline bool typeIs(BattleType value, std::initializer_list<BattleType> choices) {
    return std::find(choices.begin(), choices.end(), value) != choices.end();
}

// Generation VI–IX single-type effectiveness chart.
inline float singleTypeEffectiveness(BattleType attack, BattleType defend) {
    switch (attack) {
    case BattleType::Normal:
        if (defend == BattleType::Ghost) return 0.0f;
        if (typeIs(defend, {BattleType::Rock, BattleType::Steel})) return 0.5f;
        break;
    case BattleType::Fire:
        if (typeIs(defend, {BattleType::Grass, BattleType::Ice, BattleType::Bug, BattleType::Steel})) return 2.0f;
        if (typeIs(defend, {BattleType::Fire, BattleType::Water, BattleType::Rock, BattleType::Dragon})) return 0.5f;
        break;
    case BattleType::Water:
        if (typeIs(defend, {BattleType::Fire, BattleType::Ground, BattleType::Rock})) return 2.0f;
        if (typeIs(defend, {BattleType::Water, BattleType::Grass, BattleType::Dragon})) return 0.5f;
        break;
    case BattleType::Electric:
        if (typeIs(defend, {BattleType::Water, BattleType::Flying})) return 2.0f;
        if (defend == BattleType::Ground) return 0.0f;
        if (typeIs(defend, {BattleType::Electric, BattleType::Grass, BattleType::Dragon})) return 0.5f;
        break;
    case BattleType::Grass:
        if (typeIs(defend, {BattleType::Water, BattleType::Ground, BattleType::Rock})) return 2.0f;
        if (typeIs(defend, {BattleType::Fire, BattleType::Grass, BattleType::Poison, BattleType::Flying, BattleType::Bug, BattleType::Dragon, BattleType::Steel})) return 0.5f;
        break;
    case BattleType::Ice:
        if (typeIs(defend, {BattleType::Grass, BattleType::Ground, BattleType::Flying, BattleType::Dragon})) return 2.0f;
        if (typeIs(defend, {BattleType::Fire, BattleType::Water, BattleType::Ice, BattleType::Steel})) return 0.5f;
        break;
    case BattleType::Fighting:
        if (typeIs(defend, {BattleType::Normal, BattleType::Ice, BattleType::Rock, BattleType::Dark, BattleType::Steel})) return 2.0f;
        if (typeIs(defend, {BattleType::Poison, BattleType::Flying, BattleType::Psychic, BattleType::Bug, BattleType::Fairy})) return 0.5f;
        if (defend == BattleType::Ghost) return 0.0f;
        break;
    case BattleType::Poison:
        if (typeIs(defend, {BattleType::Grass, BattleType::Fairy})) return 2.0f;
        if (typeIs(defend, {BattleType::Poison, BattleType::Ground, BattleType::Rock, BattleType::Ghost})) return 0.5f;
        if (defend == BattleType::Steel) return 0.0f;
        break;
    case BattleType::Ground:
        if (typeIs(defend, {BattleType::Fire, BattleType::Electric, BattleType::Poison, BattleType::Rock, BattleType::Steel})) return 2.0f;
        if (typeIs(defend, {BattleType::Grass, BattleType::Bug})) return 0.5f;
        if (defend == BattleType::Flying) return 0.0f;
        break;
    case BattleType::Flying:
        if (typeIs(defend, {BattleType::Grass, BattleType::Fighting, BattleType::Bug})) return 2.0f;
        if (typeIs(defend, {BattleType::Electric, BattleType::Rock, BattleType::Steel})) return 0.5f;
        break;
    case BattleType::Psychic:
        if (typeIs(defend, {BattleType::Fighting, BattleType::Poison})) return 2.0f;
        if (typeIs(defend, {BattleType::Psychic, BattleType::Steel})) return 0.5f;
        if (defend == BattleType::Dark) return 0.0f;
        break;
    case BattleType::Bug:
        if (typeIs(defend, {BattleType::Grass, BattleType::Psychic, BattleType::Dark})) return 2.0f;
        if (typeIs(defend, {BattleType::Fire, BattleType::Fighting, BattleType::Poison, BattleType::Flying, BattleType::Ghost, BattleType::Steel, BattleType::Fairy})) return 0.5f;
        break;
    case BattleType::Rock:
        if (typeIs(defend, {BattleType::Fire, BattleType::Ice, BattleType::Flying, BattleType::Bug})) return 2.0f;
        if (typeIs(defend, {BattleType::Fighting, BattleType::Ground, BattleType::Steel})) return 0.5f;
        break;
    case BattleType::Ghost:
        if (typeIs(defend, {BattleType::Psychic, BattleType::Ghost})) return 2.0f;
        if (defend == BattleType::Dark) return 0.5f;
        if (defend == BattleType::Normal) return 0.0f;
        break;
    case BattleType::Dragon:
        if (defend == BattleType::Dragon) return 2.0f;
        if (defend == BattleType::Steel) return 0.5f;
        if (defend == BattleType::Fairy) return 0.0f;
        break;
    case BattleType::Dark:
        if (typeIs(defend, {BattleType::Psychic, BattleType::Ghost})) return 2.0f;
        if (typeIs(defend, {BattleType::Fighting, BattleType::Dark, BattleType::Fairy})) return 0.5f;
        break;
    case BattleType::Steel:
        if (typeIs(defend, {BattleType::Ice, BattleType::Rock, BattleType::Fairy})) return 2.0f;
        if (typeIs(defend, {BattleType::Fire, BattleType::Water, BattleType::Electric, BattleType::Steel})) return 0.5f;
        break;
    case BattleType::Fairy:
        if (typeIs(defend, {BattleType::Fighting, BattleType::Dragon, BattleType::Dark})) return 2.0f;
        if (typeIs(defend, {BattleType::Fire, BattleType::Poison, BattleType::Steel})) return 0.5f;
        break;
    }
    return 1.0f;
}

inline float typeEffectiveness(BattleType attack, BattleType defend) {
    return singleTypeEffectiveness(attack, defend);
}

inline float typeEffectiveness(BattleType attack, BattleType defend1,
                               BattleType defend2) {
    return singleTypeEffectiveness(attack, defend1) *
           singleTypeEffectiveness(attack, defend2);
}

} // namespace ash_horizon
