/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "battle_format.hpp"
#include "inventory.hpp"
#include "move_data.hpp"
#include "player_profile.hpp"
#include "pokemon_party.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <string>

namespace ash_horizon {

struct SaveGameData {
    SaveBattleRules battleRules{};
    PlayerProfile profile{};
    PlayerWallet wallet{};
    Inventory inventory{};
    ArceusBallState arceusBall{};
    bool privateSamfr2xActive = false;
    float playerX = 80.0f;
    float playerY = 128.0f;
};

class SaveGameStore {
public:
    static bool save(const std::string& path, const SaveGameData& data,
                     const PokemonParty& party) {
        std::ofstream out(path, std::ios::trunc);
        if (!out) return false;
        out << "ASH_HORIZON_SAVE 4\n";
        out << std::quoted(data.battleRules.serialize()) << '\n';
        out << std::quoted(data.profile.name()) << ' '
            << static_cast<unsigned>(data.profile.gender()) << '\n';
        out << data.wallet.pokeDollars() << ' ' << data.wallet.hc() << ' '
            << data.wallet.shards() << '\n';
        out << std::setprecision(9) << data.playerX << ' ' << data.playerY << '\n';
        out << (data.arceusBall.unlimitedTime ? 1 : 0) << ' '
            << data.arceusBall.remainingMilliseconds << ' '
            << (data.arceusBall.inUse ? 1 : 0) << ' '
            << (data.privateSamfr2xActive ? 1 : 0) << '\n';
        for (std::uint16_t quantity : data.inventory.quantities())
            out << quantity << ' ';
        out << '\n' << party.size() << ' ' << party.activeIndex() << '\n';
        for (const PartyPokemon& pokemon : party.members()) {
            out << pokemon.nationalDex << ' ' << static_cast<unsigned>(pokemon.level)
                << ' ' << pokemon.currentHp << ' ' << pokemon.maxHp << ' '
                << static_cast<unsigned>(pokemon.friendship) << ' '
                << pokemon.experience << ' ' << std::quoted(pokemon.nickname) << '\n';
            out << static_cast<unsigned>(pokemon.moveCount);
            for (std::size_t move = 0; move < 4; ++move)
                out << ' ' << pokemon.moveIds[move] << ' '
                    << static_cast<unsigned>(pokemon.movePp[move]);
            out << '\n';
        }
        return static_cast<bool>(out);
    }

    static bool load(const std::string& path, SaveGameData& data,
                     PokemonParty& party, const PokemonDatabase& database) {
        std::ifstream in(path);
        if (!in) return false;

        std::string header, serializedRules;
        if (!std::getline(in, header) ||
            (header != "ASH_HORIZON_SAVE 1" && header != "ASH_HORIZON_SAVE 2" &&
             header != "ASH_HORIZON_SAVE 3" && header != "ASH_HORIZON_SAVE 4"))
            return false;
        const bool hasExperience = header == "ASH_HORIZON_SAVE 2" ||
                                   header == "ASH_HORIZON_SAVE 3" ||
                                   header == "ASH_HORIZON_SAVE 4";
        const bool hasMoves = header == "ASH_HORIZON_SAVE 3" ||
                              header == "ASH_HORIZON_SAVE 4";
        const bool hasProfileAndWallet = header == "ASH_HORIZON_SAVE 4";
        if (!(in >> std::quoted(serializedRules))) return false;

        SaveGameData loaded;
        if (!SaveBattleRules::deserialize(serializedRules, loaded.battleRules)) return false;
        if (hasProfileAndWallet) {
            std::string playerName;
            unsigned gender = 0;
            std::uint32_t pokeDollars = 0, hc = 0, shards = 0;
            if (!(in >> std::quoted(playerName) >> gender >> pokeDollars >> hc >> shards) ||
                !loaded.profile.restore(playerName, gender) ||
                !loaded.wallet.restore(pokeDollars, hc, shards)) return false;
        }

        int unlimited = 0, inUse = 0, privateCheat = 0;
        if (!(in >> loaded.playerX >> loaded.playerY >> unlimited
                 >> loaded.arceusBall.remainingMilliseconds >> inUse >> privateCheat)) return false;
        if ((unlimited != 0 && unlimited != 1) || (inUse != 0 && inUse != 1) ||
            (privateCheat != 0 && privateCheat != 1)) return false;
        loaded.arceusBall.unlimitedTime = unlimited != 0;
        loaded.arceusBall.inUse = inUse != 0;
        loaded.privateSamfr2xActive = privateCheat != 0;

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
            std::uint32_t experience = 0;
            std::string nickname;
            if (!(in >> dex >> level >> hp >> maxHp >> friendship) ||
                (hasExperience && !(in >> experience)) || !(in >> std::quoted(nickname)) ||
                dex > 1025 || level == 0 || level > 100 || friendship > 100 ||
                maxHp <= 0 || hp < 0 || hp > maxHp ||
                !loadedParty.add(database, static_cast<std::uint16_t>(dex),
                                 static_cast<std::uint8_t>(level), nickname) ||
                !loadedParty.restoreMemberState(i, hp, maxHp,
                    static_cast<std::uint8_t>(friendship))) return false;
            loadedParty.member(i)->experience = experience;
            initializeMoveSet(*loadedParty.member(i));
            if (hasMoves) {
                unsigned moveCount = 0;
                std::array<std::uint16_t, 4> moveIds{{0, 0, 0, 0}};
                std::array<std::uint8_t, 4> movePp{{0, 0, 0, 0}};
                if (!(in >> moveCount) || moveCount > 4) return false;
                for (std::size_t move = 0; move < 4; ++move) {
                    unsigned id = 0, pp = 0;
                    if (!(in >> id >> pp) || id > 65535 || pp > 255) return false;
                    if (move < moveCount) {
                        const MoveDefinition* definition = findMoveDefinition(id);
                        if (!definition || pp > definition->maxPp) return false;
                    } else if (id != 0 || pp != 0) {
                        return false;
                    }
                    moveIds[move] = static_cast<std::uint16_t>(id);
                    movePp[move] = static_cast<std::uint8_t>(pp);
                }
                if (!loadedParty.restoreMoveState(i, static_cast<std::uint8_t>(moveCount),
                                                  moveIds, movePp)) return false;
            }
        }
        if (!loadedParty.restoreActiveIndex(activeIndex)) return false;

        data = loaded;
        party = loadedParty;
        return true;
    }
};

} // namespace ash_horizon
