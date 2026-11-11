/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>

namespace ash_horizon {

enum class BattleStyle : std::uint8_t { Single = 0, Double = 1 };

// Stored once per save file. A one-Pokémon party is always forced to Single.
struct SaveBattleRules {
    std::uint32_t seed = 0;
    BattleStyle preferredStyle = BattleStyle::Single;

    static SaveBattleRules create(std::uint32_t saveSeed) {
        std::mt19937 rng(saveSeed);
        std::uniform_int_distribution<int> choose(0, 1);
        return {saveSeed, choose(rng) == 0 ? BattleStyle::Single : BattleStyle::Double};
    }

    BattleStyle resolve(std::size_t usablePlayerPokemon) const {
        return usablePlayerPokemon < 2 ? BattleStyle::Single : preferredStyle;
    }

    std::size_t opponentTeamSize(std::size_t playerPokemonCount) const {
        return std::min<std::size_t>(playerPokemonCount, 6);
    }

    std::size_t activePokemonPerSide(std::size_t usablePlayerPokemon) const {
        return resolve(usablePlayerPokemon) == BattleStyle::Double ? 2u : 1u;
    }

    std::string serialize() const {
        std::ostringstream out;
        out << "AH_BATTLE_RULES 1\n"
            << "seed=" << seed << "\n"
            << "style=" << (preferredStyle == BattleStyle::Double ? "double" : "single")
            << "\n";
        return out.str();
    }

    static bool deserialize(const std::string& data, SaveBattleRules& output) {
        std::istringstream in(data);
        std::string header, seedLine, styleLine;
        if (!std::getline(in, header) || header != "AH_BATTLE_RULES 1" ||
            !std::getline(in, seedLine) || seedLine.rfind("seed=", 0) != 0 ||
            !std::getline(in, styleLine) || styleLine.rfind("style=", 0) != 0) {
            return false;
        }

        try {
            const unsigned long parsedSeed = std::stoul(seedLine.substr(5));
            if (parsedSeed > 0xFFFFFFFFul) return false;
            BattleStyle parsedStyle;
            const std::string value = styleLine.substr(6);
            if (value == "single") parsedStyle = BattleStyle::Single;
            else if (value == "double") parsedStyle = BattleStyle::Double;
            else return false;
            output.seed = static_cast<std::uint32_t>(parsedSeed);
            output.preferredStyle = parsedStyle;
            return true;
        } catch (...) {
            return false;
        }
    }
};

} // namespace ash_horizon
