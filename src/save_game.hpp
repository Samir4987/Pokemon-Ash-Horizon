/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "battle_format.hpp"
#include "inventory.hpp"
#include "pokemon_party.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <string>

namespace ash_horizon {

struct SaveGameData {
    SaveBattleRules battleRules{};
    Inventory inventory{};
    ArceusBallState arceusBall{};
    float playerX = 80.0f;
    float playerY = 128.0f;
};

class SaveGameStore {
public:
    static bool save(const std::string& path, const SaveGameData& data,
                     const PokemonParty& party) {
        std::ofstream out(path, std::ios::trunc);
        if (!out) return false;
        out << "ASH_HORIZON_SAVE 1\n";
        out << std::quoted(data.battleRules.serialize()) << '\n';
        out << std::setprecision(9) << data.playerX << ' ' << data.playerY << '\n';
        out << (data.arceusBall.unlimitedTime ? 1 : 0) << ' '
            << data.arceusBall.remainingMilliseconds << ' '
            << (data.arceusBall.inUse ? 1 : 0) << '\n';
        for (std::uint16_t quantity : data.inventory.quantities())
            out << quantity << ' ';
        out << '\n' << party.size() << ' ' << party.activeIndex() << '\n';
        for (const PartyPokemon& pokemon : party.members()) {
            out << pokemon.nationalDex << ' ' << static_cast<unsigned>(pokemon.level)
                << ' ' << pokemon.currentHp << ' ' << pokemon.maxHp << ' '
                << static_cast<unsigned>(pokemon.friendship) << ' '
                << std::quoted(pokemon.nickname) << '\n';
        }
        return static_cast<bool>(out);
    }

    static bool load(const std::string& path, SaveGameData& data,
                     PokemonParty& party, const PokemonDatabase& database) {
        std::ifstream in(path);
        if (!in) return false;

        std::string header, serializedRules;
        if (!std::getline(in, header) || header != "ASH_HORIZON_SAVE 1") return false;
        if (!(in >> std::quoted(serializedRules))) return false;

        SaveGameData loaded;
        if (!SaveBattleRules::deserialize(serializedRules, loaded.battleRules)) return false;
        int unlimited = 0, inUse = 0;
        if (!(in >> loaded.playerX >> loaded.playerY >> unlimited
                 >> loaded.arceusBall.remainingMilliseconds >> inUse)) return false;
        if ((unlimited != 0 && unlimited != 1) || (inUse != 0 && inUse != 1)) return false;
        loaded.arceusBall.unlimitedTime = unlimited != 0;
        loaded.arceusBall.inUse = inUse != 0;

        for (std::size_t i = 0; i < Inventory::ITEM_COUNT; ++i) {
            unsigned quantity = 0;
            if (!(in >> quantity) || quantity > Inventory::MAX_STACK) return false;
            loaded.inventory.set(static_cast<ItemId>(i),
                                 static_cast<std::uint16_t>(quantity));
        }

        std::size_t partyCount = 0, activeIndex = 0;
        if (!(in >> partyCount >> activeIndex) ||
            partyCount > PokemonParty::MAX_SIZE ||
            (partyCount == 0 ? activeIndex != 0 : activeIndex >= partyCount)) return false;

        PokemonParty loadedParty;
        for (std::size_t i = 0; i < partyCount; ++i) {
            unsigned dex = 0, level = 0, friendship = 0;
            int hp = 0, maxHp = 0;
            std::string nickname;
            if (!(in >> dex >> level >> hp >> maxHp >> friendship >> std::quoted(nickname)) ||
                dex > 1025 || level == 0 || level > 100 || friendship > 100 ||
                maxHp <= 0 || hp < 0 || hp > maxHp ||
                !loadedParty.add(database, static_cast<std::uint16_t>(dex),
                                 static_cast<std::uint8_t>(level), nickname) ||
                !loadedParty.restoreMemberState(i, hp, maxHp,
                    static_cast<std::uint8_t>(friendship))) return false;
        }
        if (partyCount > 0 && activeIndex != 0 && !loadedParty.switchTo(activeIndex))
            return false;

        data = loaded;
        party = loadedParty;
        return true;
    }
};

} // namespace ash_horizon
