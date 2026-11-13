/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "../src/healing_system.hpp"
#include <cassert>

int main() {
    using namespace ash_horizon;
    PokemonDatabase database;
    SpeciesRecord species;
    species.nationalDex = 1;
    species.name = "Bulbasaur";
    species.baseStats.hp = 45;
    assert(database.addSpecies(species));

    PokemonParty party;
    assert(party.add(database, 1, 5));
    assert(party.add(database, 1, 5));
    party.damageActive(999);
    float x = 200.0f, y = 180.0f;
    assert(!HealingSystem::allFainted(party));
    assert(!HealingSystem::emergencyReturn(party, x, y, 64.0f, 96.0f));
    assert(x == 200.0f && y == 180.0f);
    assert(party.switchTo(1));
    party.damageActive(999);
    assert(HealingSystem::allFainted(party));
    assert(HealingSystem::emergencyReturn(party, x, y, 64.0f, 96.0f));
    assert(x == 64.0f && y == 96.0f);
    assert(party.usableCount() == 2);
    assert(party.member(0)->currentHp == party.member(0)->maxHp);
    assert(party.member(1)->currentHp == party.member(1)->maxHp);

    party.damageActive(5);
    assert(HealingSystem::healAtCenter(party));
    assert(party.active()->currentHp == party.active()->maxHp);
    PokemonParty empty;
    assert(!HealingSystem::healAtCenter(empty));
    assert(!HealingSystem::allFainted(empty));
    return 0;
}
