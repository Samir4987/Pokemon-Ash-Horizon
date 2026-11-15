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

#include "pokemon_data.hpp"
#include "pokemon_genetics.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ash_horizon {

struct PartyPokemon {
    std::uint16_t nationalDex = 0;
    std::string speciesName;
    std::string nickname;
    std::uint8_t level = 1;
    int currentHp = 1;
    int maxHp = 1;
    std::uint8_t friendship = 100;
    std::uint32_t experience = 0;
    std::array<std::uint8_t, 6> ivs{{0, 0, 0, 0, 0, 0}};
    std::array<std::uint16_t, 6> evs{{0, 0, 0, 0, 0, 0}};
    NatureId nature = NatureId::Hardy;
    std::array<std::uint16_t, 4> moveIds{{0, 0, 0, 0}};
    std::array<std::uint8_t, 4> movePp{{0, 0, 0, 0}};
    std::uint8_t moveCount = 0;

    bool fainted() const { return currentHp <= 0; }
};

class PokemonParty {
public:
    static constexpr std::size_t MAX_SIZE = 6;

    bool add(const PokemonDatabase& database, std::uint16_t nationalDex,
             std::uint8_t level, const std::string& nickname = {}) {
        const SpeciesRecord* species = database.findSpecies(nationalDex);
        if (!species || level == 0 || members_.size() >= MAX_SIZE) return false;

        PartyPokemon pokemon;
        pokemon.nationalDex = nationalDex;
        pokemon.speciesName = species->name;
        pokemon.nickname = nickname;
        pokemon.level = level;
        StatInputs statInputs;
        statInputs.level = level;
        statInputs.baseHp = species->baseStats.hp;
        statInputs.baseAttack = species->baseStats.attack;
        statInputs.baseDefense = species->baseStats.defense;
        statInputs.baseSpecialAttack = species->baseStats.specialAttack;
        statInputs.baseSpecialDefense = species->baseStats.specialDefense;
        statInputs.baseSpeed = species->baseStats.speed;
        pokemon.maxHp = calculateBattleStats(statInputs).hp;
        pokemon.currentHp = pokemon.maxHp;
        members_.push_back(pokemon);
        if (members_.size() == 1) activeIndex_ = 0;
        return true;
    }

    bool restoreMemberState(std::size_t index, int hp, int maxHp,
                            std::uint8_t friendship) {
        if (index >= members_.size() || maxHp <= 0 || hp < 0 ||
            hp > maxHp || friendship > 100) return false;
        members_[index].currentHp = hp;
        members_[index].maxHp = maxHp;
        members_[index].friendship = friendship;
        return true;
    }

    bool restoreGenetics(std::size_t index,
                         const std::array<std::uint8_t, 6>& ivs,
                         const std::array<std::uint16_t, 6>& evs,
                         std::uint8_t nature) {
        if (index >= members_.size() || nature >= NATURE_COUNT) return false;
        std::array<int, 6> ivValues{}, evValues{};
        for (std::size_t i = 0; i < 6; ++i) {
            ivValues[i] = ivs[i];
            evValues[i] = evs[i];
        }
        if (!isValidIvSpread(ivValues) || !isValidEvSpread(evValues)) return false;
        members_[index].ivs = ivs;
        members_[index].evs = evs;
        members_[index].nature = static_cast<NatureId>(nature);
        return true;
    }

    BattleStats battleStats(std::size_t index, const PokemonDatabase& database) const {
        if (index >= members_.size()) return {};
        const PartyPokemon& pokemon = members_[index];
        const SpeciesRecord* species = database.findSpecies(pokemon.nationalDex);
        if (!species) return {};
        StatInputs inputs;
        inputs.level = pokemon.level;
        inputs.baseHp = species->baseStats.hp;
        inputs.baseAttack = species->baseStats.attack;
        inputs.baseDefense = species->baseStats.defense;
        inputs.baseSpecialAttack = species->baseStats.specialAttack;
        inputs.baseSpecialDefense = species->baseStats.specialDefense;
        inputs.baseSpeed = species->baseStats.speed;
        for (std::size_t i = 0; i < 6; ++i) {
            inputs.ivs[i] = pokemon.ivs[i];
            inputs.evs[i] = pokemon.evs[i];
        }
        inputs.nature = natureFromId(static_cast<std::uint8_t>(pokemon.nature));
        return calculateBattleStats(inputs);
    }

    bool restoreMoveState(std::size_t index, std::uint8_t count,
                          const std::array<std::uint16_t, 4>& ids,
                          const std::array<std::uint8_t, 4>& pp) {
        if (index >= members_.size() || count > 4) return false;
        members_[index].moveCount = count;
        members_[index].moveIds = ids;
        members_[index].movePp = pp;
        return true;
    }

    bool restoreActiveIndex(std::size_t index) {
        if (index >= members_.size()) return members_.empty() && index == 0;
        activeIndex_ = index;
        return true;
    }

    void clear() {
        members_.clear();
        activeIndex_ = 0;
    }

    std::size_t size() const { return members_.size(); }
    bool empty() const { return members_.empty(); }
    std::size_t usableCount() const {
        return static_cast<std::size_t>(std::count_if(members_.begin(), members_.end(),
            [](const PartyPokemon& pokemon) { return !pokemon.fainted(); }));
    }
    std::size_t activeIndex() const { return activeIndex_; }
    const std::vector<PartyPokemon>& members() const { return members_; }

    PartyPokemon* member(std::size_t index) {
        return index < members_.size() ? &members_[index] : nullptr;
    }

    const PartyPokemon* member(std::size_t index) const {
        return index < members_.size() ? &members_[index] : nullptr;
    }

    PartyPokemon* active() {
        return empty() ? nullptr : &members_[activeIndex_];
    }
    const PartyPokemon* active() const {
        return empty() ? nullptr : &members_[activeIndex_];
    }

    bool switchTo(std::size_t index) {
        if (index >= members_.size() || index == activeIndex_ ||
            members_[index].fainted()) return false;
        activeIndex_ = index;
        return true;
    }

    void damageActive(int damage) {
        PartyPokemon* pokemon = active();
        if (!pokemon || damage <= 0) return;
        pokemon->currentHp = std::max(0, pokemon->currentHp - damage);
    }

    void healAll() {
        for (auto& pokemon : members_) pokemon.currentHp = pokemon.maxHp;
    }

private:
    std::vector<PartyPokemon> members_;
    std::size_t activeIndex_ = 0;
};

} // namespace ash_horizon
