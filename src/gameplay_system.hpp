/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "inventory.hpp"
#include "move_data.hpp"
#include "pokemon_party.hpp"
#include "type_chart.hpp"
#include <algorithm>
#include <cstdint>
#include <random>

namespace ash_horizon {

// Small, deterministic gameplay rules kept separate from rendering and SDL.
struct StarterChoice {
    std::uint16_t dex;
    const char* name;
};

class StarterSelection {
public:
    static constexpr StarterChoice OPTIONS[3] = {
        {1, "Bulbasaur"}, {4, "Charmander"}, {7, "Squirtle"}
    };

    static bool choose(std::size_t index, const PokemonDatabase& database,
                       PokemonParty& party, std::uint8_t level = 5) {
        if (index >= 3 || !party.empty()) return false;
        if (!party.add(database, OPTIONS[index].dex, level)) return false;
        initializeMoveSet(*party.member(party.size() - 1));
        return true;
    }
};

enum class CaptureResult : std::uint8_t {
    Caught, Failed, NoBall, PartyFull, InvalidTarget, TimerExpired
};

class CaptureSystem {
public:
    static int chancePercent(ItemId ball, int currentHp, int maxHp,
                             int catchRate = 45) {
        if (maxHp <= 0 || currentHp <= 0 || currentHp > maxHp) return 0;
        if (ball == ItemId::MasterBall) return 100;
        int bonus = 1;
        if (ball == ItemId::GreatBall) bonus = 2;
        else if (ball == ItemId::UltraBall || ball == ItemId::ArceusBall) bonus = 3;
        else if (ball != ItemId::PokeBall) return 0;
        const int numerator = catchRate * bonus * (3 * maxHp - 2 * currentHp);
        const int denominator = std::max(1, 3 * maxHp);
        return std::clamp(numerator / denominator, 1, 100);
    }

    static CaptureResult attempt(ItemId ball, Inventory& inventory,
                                 ArceusBallState& arceusTimer,
                                 PokemonParty& party,
                                 const PokemonDatabase& database,
                                 std::uint16_t targetDex, int currentHp,
                                 int maxHp, int rollPercent) {
        if (!database.findSpecies(targetDex) || currentHp <= 0 || maxHp <= 0 ||
            currentHp > maxHp) return CaptureResult::InvalidTarget;
        if (party.size() >= PokemonParty::MAX_SIZE) return CaptureResult::PartyFull;
        if (inventory.quantity(ball) == 0) return CaptureResult::NoBall;
        if (ball == ItemId::ArceusBall && arceusTimer.expired())
            return CaptureResult::TimerExpired;
        if (ball == ItemId::ArceusBall && !arceusTimer.inUse)
            arceusTimer.startCountdown(30000);
        inventory.remove(ball);
        if (std::clamp(rollPercent, 1, 100) > chancePercent(ball, currentHp, maxHp))
            return CaptureResult::Failed;
        if (!party.add(database, targetDex, 5)) return CaptureResult::PartyFull;
        initializeMoveSet(*party.member(party.size() - 1));
        if (ball == ItemId::ArceusBall) arceusTimer.stop();
        return CaptureResult::Caught;
    }
};

class ProgressionSystem {
public:
    static std::uint32_t experienceForNextLevel(std::uint8_t level) {
        const std::uint32_t safe = std::max<unsigned>(1, level);
        return safe * safe * safe;
    }

    static bool award(PokemonParty& party, std::size_t memberIndex,
                      const PokemonDatabase& database, std::uint32_t amount) {
        PartyPokemon* pokemon = party.member(memberIndex);
        if (!pokemon || pokemon->level >= 100 || amount == 0) return false;
        const SpeciesRecord* species = database.findSpecies(pokemon->nationalDex);
        if (!species) return false;
        pokemon->experience += amount;
        bool levelled = false;
        while (pokemon->level < 100 &&
               pokemon->experience >= experienceForNextLevel(pokemon->level)) {
            pokemon->experience -= experienceForNextLevel(pokemon->level);
            ++pokemon->level;
            const int oldMax = pokemon->maxHp;
            pokemon->maxHp = std::max(1, ((2 * species->baseStats.hp * pokemon->level) / 100)
                                          + pokemon->level + 10);
            pokemon->currentHp = std::min(pokemon->maxHp,
                pokemon->currentHp + (pokemon->maxHp - oldMax));
            levelled = true;
        }
        if (pokemon->level == 100)
            pokemon->experience = 0;
        return levelled;
    }
};

class ItemUseSystem {
public:
    static bool useHealingItem(ItemId item, Inventory& inventory,
                               PokemonParty& party, std::size_t targetIndex) {
        PartyPokemon* target = party.member(targetIndex);
        if (!target || target->fainted()) return false;
        int healing = 0;
        switch (item) {
        case ItemId::Potion: healing = 20; break;
        case ItemId::SuperPotion: healing = 60; break;
        case ItemId::HyperPotion: healing = 120; break;
        case ItemId::MaxPotion: healing = target->maxHp; break;
        case ItemId::FullRestore: healing = target->maxHp; break;
        default: return false;
        }
        if (target->currentHp >= target->maxHp || inventory.quantity(item) == 0)
            return false;
        if (!inventory.remove(item)) return false;
        target->currentHp = std::min(target->maxHp, target->currentHp + healing);
        return true;
    }

    static bool useRevive(ItemId item, Inventory& inventory,
                          PokemonParty& party, std::size_t targetIndex) {
        PartyPokemon* target = party.member(targetIndex);
        if (!target || !target->fainted() ||
            (item != ItemId::Revive && item != ItemId::MaxRevive) ||
            inventory.quantity(item) == 0) return false;
        if (!inventory.remove(item)) return false;
        target->currentHp = item == ItemId::MaxRevive
            ? target->maxHp : std::max(1, target->maxHp / 2);
        return true;
    }
};

} // namespace ash_horizon
