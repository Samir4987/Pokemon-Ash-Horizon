/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "type_chart.hpp"
#include <array>
#include <cstdint>

namespace ash_horizon {

enum class MoveClass : std::uint8_t { Physical, Special, Status };
enum class MoveId : std::uint16_t {
    Tackle = 1, Growl, VineWhip, RazorLeaf, Scratch,
    Ember, WaterGun, TailWhip, Bubble, ThunderShock
};

struct MoveDefinition {
    MoveId id;
    const char* name;
    BattleType type;
    MoveClass category;
    std::uint8_t power;
    std::uint8_t accuracy;
    std::uint8_t maxPp;
};

inline const MoveDefinition* findMoveDefinition(std::uint16_t rawId) {
    static constexpr std::array<MoveDefinition, 10> moves{{
        {MoveId::Tackle, "Tackle", BattleType::Normal, MoveClass::Physical, 40, 100, 35},
        {MoveId::Growl, "Growl", BattleType::Normal, MoveClass::Status, 0, 100, 40},
        {MoveId::VineWhip, "Vine Whip", BattleType::Grass, MoveClass::Physical, 45, 100, 25},
        {MoveId::RazorLeaf, "Razor Leaf", BattleType::Grass, MoveClass::Physical, 55, 95, 25},
        {MoveId::Scratch, "Scratch", BattleType::Normal, MoveClass::Physical, 40, 100, 35},
        {MoveId::Ember, "Ember", BattleType::Fire, MoveClass::Special, 40, 100, 25},
        {MoveId::WaterGun, "Water Gun", BattleType::Water, MoveClass::Special, 40, 100, 25},
        {MoveId::TailWhip, "Tail Whip", BattleType::Normal, MoveClass::Status, 0, 100, 30},
        {MoveId::Bubble, "Bubble", BattleType::Water, MoveClass::Special, 40, 100, 30},
        {MoveId::ThunderShock, "Thunder Shock", BattleType::Electric, MoveClass::Special, 40, 100, 30}
    }};
    for (const auto& move : moves)
        if (static_cast<std::uint16_t>(move.id) == rawId) return &move;
    return nullptr;
}

inline std::array<std::uint16_t, 4> defaultMoveIds(std::uint16_t dex) {
    switch (dex) {
    case 1: return {{1, 2, 3, 4}};
    case 4: return {{5, 2, 6, 1}};
    case 7: return {{1, 8, 7, 9}};
    default: return {{1, 0, 0, 0}};
    }
}

} // namespace ash_horizon
