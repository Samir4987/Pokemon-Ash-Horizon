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

    SpeciesRecord charizard=bulba; charizard.nationalDex=6; charizard.name="Charizard";
    charizard.type1=PokemonType::Fire; assert(db.addSpecies(charizard));

    PokemonParty party;
    assert(StarterSelection::choose(1,db,party));
    assert(party.size()==1 && party.active()->nationalDex==4);
    assert(party.active()->moveCount == 4);
    PokemonParty friendshipParty;
    assert(StarterSelection::choose(1,db,friendshipParty));
    assert(addFriendshipTestTeam(db,friendshipParty));
    assert(friendshipParty.size()==3);
    assert(friendshipParty.member(0)->nationalDex==4 && friendshipParty.member(0)->friendship==98);
    assert(friendshipParty.member(1)->nationalDex==1 && friendshipParty.member(1)->friendship==46);
    assert(friendshipParty.member(2)->nationalDex==7 && friendshipParty.member(2)->friendship==22);
    assert(friendshipParty.member(1)->moveCount==4 && friendshipParty.member(2)->moveCount==4);
    assert(!addFriendshipTestTeam(db,friendshipParty));
    assert(party.active()->moveIds[0] == static_cast<std::uint16_t>(MoveId::Scratch));
    assert(party.active()->movePp[0] == 35);
    party.active()->movePp[0] = 14;
    assert(!StarterSelection::choose(0,db,party));

    Inventory bag; bag.add(ItemId::PokeBall,2); bag.add(ItemId::Potion,1);
    ArceusBallState timer;
    const int maxHp=40;
    assert(CaptureSystem::chancePercent(ItemId::PokeBall,10,maxHp)> 
           CaptureSystem::chancePercent(ItemId::PokeBall,35,maxHp));
    assert(CaptureSystem::attempt(ItemId::PokeBall,bag,timer,party,db,1,5,maxHp,1)
           ==CaptureResult::Caught);
    assert(party.size()==2 && bag.quantity(ItemId::PokeBall)==1);
    assert(party.member(1)->moveCount == 4);
    assert(party.member(1)->moveIds[2] == static_cast<std::uint16_t>(MoveId::VineWhip));
    assert(CaptureSystem::chancePercent(ItemId::MasterBall,40,40)==100);
    bag.add(ItemId::ArceusBall,1);
    ArceusBallState timedBall;
    assert(CaptureSystem::attempt(ItemId::ArceusBall,bag,timedBall,party,db,7,40,40,100)
           ==CaptureResult::Failed);
    assert(timedBall.inUse && timedBall.remainingMilliseconds==30000);
    timedBall.update(30000);
    assert(timedBall.expired());
    bag.add(ItemId::ArceusBall,1);
    assert(CaptureSystem::attempt(ItemId::ArceusBall,bag,timedBall,party,db,7,5,40,1)
           ==CaptureResult::TimerExpired);

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
    assert(loadedParty.member(0)->moveCount == party.member(0)->moveCount);
    assert(loadedParty.member(0)->moveIds == party.member(0)->moveIds);
    assert(loadedParty.member(0)->movePp == party.member(0)->movePp);
    assert(loadedParty.member(0)->movePp[0] == 14);
    std::remove(path);
    return 0;
}
