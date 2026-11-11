/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "../src/gameplay_system.hpp"
#include "../src/save_game.hpp"
#include <cassert>
#include <cstdio>

int main() {
    using namespace ash_horizon;
    PokemonDatabase db;
    SpeciesRecord bulba;
    bulba.nationalDex=1; bulba.name="Bulbasaur"; bulba.type1=PokemonType::Grass;
    bulba.baseStats.hp=45; bulba.baseStats.attack=49; bulba.baseStats.defense=49;
    bulba.baseStats.specialAttack=65; bulba.baseStats.specialDefense=65; bulba.baseStats.speed=45;
    assert(db.addSpecies(bulba));
    SpeciesRecord charmander=bulba; charmander.nationalDex=4; charmander.name="Charmander";
    charmander.type1=PokemonType::Fire; assert(db.addSpecies(charmander));
    SpeciesRecord squirtle=bulba; squirtle.nationalDex=7; squirtle.name="Squirtle";
    squirtle.type1=PokemonType::Water; assert(db.addSpecies(squirtle));

    PokemonParty party;
    assert(StarterSelection::choose(1,db,party));
    assert(party.size()==1 && party.active()->nationalDex==4);
    assert(!StarterSelection::choose(0,db,party));

    Inventory bag; bag.add(ItemId::PokeBall,2); bag.add(ItemId::Potion,1);
    ArceusBallState timer;
    const int maxHp=40;
    assert(CaptureSystem::chancePercent(ItemId::PokeBall,10,maxHp)> 
           CaptureSystem::chancePercent(ItemId::PokeBall,35,maxHp));
    assert(CaptureSystem::attempt(ItemId::PokeBall,bag,timer,party,db,1,5,maxHp,1)
           ==CaptureResult::Caught);
    assert(party.size()==2 && bag.quantity(ItemId::PokeBall)==1);

    party.member(0)->currentHp=1;
    assert(ItemUseSystem::useHealingItem(ItemId::Potion,bag,party,0));
    assert(party.member(0)->currentHp>1 && bag.quantity(ItemId::Potion)==0);
    party.member(0)->currentHp=0;
    bag.add(ItemId::Revive,1);
    assert(ItemUseSystem::useRevive(ItemId::Revive,bag,party,0));
    assert(party.member(0)->currentHp>0);

    const auto oldLevel=party.member(0)->level;
    assert(ProgressionSystem::award(party,0,db,5000));
    assert(party.member(0)->level>oldLevel);
    assert(party.member(0)->experience<ProgressionSystem::experienceForNextLevel(
        party.member(0)->level));

    const char* path="gameplay_save_test.tmp";
    SaveGameData data;
    assert(SaveGameStore::save(path,data,party));
    SaveGameData loaded; PokemonParty loadedParty;
    assert(SaveGameStore::load(path,loaded,loadedParty,db));
    assert(loadedParty.member(0)->experience==party.member(0)->experience);
    assert(loadedParty.member(0)->level==party.member(0)->level);
    std::remove(path);
    return 0;
}
