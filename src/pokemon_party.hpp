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
#include <algorithm>
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
        // Simplified HP formula with IVs and EVs set to zero for this prototype.
        pokemon.maxHp = std::max(1, ((2 * species->baseStats.hp * level) / 100)
                                    + level + 10);
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
