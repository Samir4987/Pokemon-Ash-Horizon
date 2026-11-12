/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "../src/save_game.hpp"
#include <cassert>
#include <cstdio>

int main() {
    using namespace ash_horizon;

    PlayerProfile profile;
    assert(profile.name() == "Trainer");
    assert(profile.gender() == PlayerGender::Unselected);
    assert(profile.setName("Samir"));
    assert(!profile.setName(""));
    assert(!profile.setName("TrainerNameTooLong"));
    assert(!profile.setName("Bad/Name"));
    assert(profile.setGender(PlayerGender::Male));
    assert(!profile.setGender(static_cast<PlayerGender>(3)));

    PlayerWallet wallet;
    assert(wallet.earnPokeDollars(30000));
    assert(wallet.earnHC(10));
    assert(wallet.earnShards(2));
    assert(wallet.spendPokeDollars(30000));
    assert(wallet.pokeDollars() == 0);
    assert(!wallet.spendPokeDollars(1));
    assert(wallet.earnPokeDollars(PlayerWallet::MAX_POKE_DOLLARS));
    assert(!wallet.earnPokeDollars(1));
    assert(wallet.pokeDollars() == PlayerWallet::MAX_POKE_DOLLARS);
    assert(wallet.restore(30000, 10, 2));
    assert(!wallet.restore(PlayerWallet::MAX_POKE_DOLLARS + 1u, 0, 0));

    PokemonDatabase database;
    SpeciesRecord species;
    species.nationalDex = 1;
    species.name = "Bulbasaur";
    species.type1 = PokemonType::Grass;
    species.baseStats.hp = 45;
    assert(database.addSpecies(species));

    PokemonParty party;
    assert(party.add(database, 1, 12, "Leaf Buddy"));
    assert(party.add(database, 1, 9, "Backup"));
    assert(party.switchTo(1));
    party.damageActive(5);
    party.active()->friendship = 72;

    SaveGameData data;
    data.battleRules = SaveBattleRules{987654u, BattleStyle::Double};
    assert(data.profile.setName("Samir"));
    assert(data.profile.setGender(PlayerGender::Male));
    assert(data.wallet.earnPokeDollars(30000));
    assert(data.wallet.earnHC(10));
    assert(data.wallet.earnShards(2));
    data.inventory.grantEverything(Inventory::MAX_STACK);
    assert(data.inventory.quantity(ItemId::MasterBall) == 999);
    assert(data.inventory.quantity(ItemId::ArceusBall) == 999);
    assert(data.inventory.remove(ItemId::Potion, 2));
    data.arceusBall.startCountdown(25000);
    data.arceusBall.update(5000);
    assert(data.arceusBall.remainingMilliseconds == 20000);
    data.arceusBall.unlimitedTime = true;
    data.privateSamfr2xActive = true;
    data.playerX = 333.5f;
    data.playerY = 121.25f;

    const char* path = "ash_horizon_save_test.tmp";
    assert(SaveGameStore::save(path, data, party));

    SaveGameData loaded;
    PokemonParty loadedParty;
    assert(SaveGameStore::load(path, loaded, loadedParty, database));
    assert(loaded.battleRules.seed == data.battleRules.seed);
    assert(loaded.battleRules.preferredStyle == BattleStyle::Double);
    assert(loaded.profile.name() == "Samir");
    assert(loaded.profile.gender() == PlayerGender::Male);
    assert(loaded.wallet.pokeDollars() == 30000);
    assert(loaded.wallet.hc() == 10);
    assert(loaded.wallet.shards() == 2);
    assert(loaded.inventory.quantity(ItemId::Potion) == 997);
    assert(loaded.inventory.quantity(ItemId::MasterBall) == 999);
    assert(loaded.inventory.quantity(ItemId::ArceusBall) == 999);
    assert(loaded.arceusBall.unlimitedTime);
    assert(loaded.privateSamfr2xActive);
    assert(loaded.arceusBall.inUse);
    assert(loaded.arceusBall.remainingMilliseconds == 20000);
    assert(loaded.playerX == data.playerX && loaded.playerY == data.playerY);
    assert(loadedParty.size() == 2);
    assert(loadedParty.activeIndex() == 1);
    assert(loadedParty.active()->nickname == "Backup");
    assert(loadedParty.active()->currentHp == loadedParty.active()->maxHp - 5);
    assert(loadedParty.active()->friendship == 72);

    ArceusBallState timer;
    timer.startCountdown(1000);
    timer.update(1000);
    assert(timer.expired());
    timer.unlimitedTime = true;
    timer.startCountdown(1000);
    timer.update(60000);
    assert(!timer.expired());
    assert(timer.remainingMilliseconds == 1000);

    std::remove(path);
    return 0;
}
